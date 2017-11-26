#include "i2c.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/twi.h>
#include "config.h"
#include "fault.h"
#include "assert.h"
#include "ring_buffer.h"


typedef enum i2c_command_code_t_ {
  I2C_COMMAND_START = 0x01,
  I2C_COMMAND_SEND_DATA = 0x02,
  I2C_COMMAND_STOP = 0x03
} i2c_command_code_t;


typedef struct i2c_command_t_ {
  i2c_command_code_t code;
  uint8_t data;
} i2c_command_t;


typedef struct i2c_command_buffer_t_ {
  uint8_t length;
  bool busy;
  i2c_command_t commands[I2C_BUFFER_SIZE];
} i2c_command_buffer_t;


typedef struct i2c_task_t_ {
  i2c_callback_t callback;
  void *data;
  uint8_t address;
} i2c_task_t;


typedef struct i2c_queue_t_ {
  i2c_command_buffer_t buffer[2];
  i2c_command_buffer_t *consumer_buffer;
  i2c_command_buffer_t *producer_buffer;
  uint8_t consumer_buffer_cursor;
  bool interrupt_pending;
  bool is_idle;
  i2c_task_t tasks_storage[I2C_QUEUE_SIZE];
  ring_buffer_t tasks;
} i2c_queue_t;


static i2c_queue_t I2C_QUEUE = { 0 };
static volatile uint16_t QUEUE_LEN = 0;


#define assert_interrupts_disabled() assert((SREG & _BV(SREG_I)) == 0)


/* Low level hardware operations -------------------------------------------- */

static inline void
i2c_hw_send_start_condition(void)
{
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
}


static inline void
i2c_hw_send_stop_condition(void)
{
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
  while (TWCR & _BV(TWSTO));
}


static inline void
i2c_hw_send_byte(uint8_t octet)
{
  TWDR = octet;
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}


static inline void
i2c_hw_go_idle(void)
{
  TWCR = 0;
}


static inline void
i2c_hw_init(void)
{
  TWSR = 0x00;
  TWCR = 0x00;
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
}

/* end of Low level hardware operations ------------------------------------- */

static void
i2c_queue_switch_buffers()
{
  assert(I2C_QUEUE.producer_buffer->busy == false);
  assert(I2C_QUEUE.producer_buffer->length > 0);
  assert(I2C_QUEUE.consumer_buffer->busy == false);
  assert(I2C_QUEUE.consumer_buffer_cursor == I2C_QUEUE.consumer_buffer->length);

  i2c_command_buffer_t *tmp = I2C_QUEUE.producer_buffer;
  I2C_QUEUE.producer_buffer = I2C_QUEUE.consumer_buffer;
  I2C_QUEUE.consumer_buffer = tmp;

  I2C_QUEUE.producer_buffer->busy = true;
  I2C_QUEUE.producer_buffer->length = 0;

  I2C_QUEUE.consumer_buffer->busy = true;
  I2C_QUEUE.consumer_buffer_cursor = 0;
}


static void
i2c_queue_switch_tasks(void)
{
  assert_interrupts_disabled();
  ring_buffer_assert_can_read_n_elements(&(I2C_QUEUE.tasks), 1);

  ring_buffer_discard(&(I2C_QUEUE.tasks));
}


static void i2c_queue_fetch_commands(void);


static void
i2c_queue_process_command(void)
{
  assert_interrupts_disabled();

  assert(I2C_QUEUE.consumer_buffer->length > 0);
  assert(I2C_QUEUE.consumer_buffer_cursor < I2C_QUEUE.consumer_buffer->length);
  assert(I2C_QUEUE.consumer_buffer->busy);

  i2c_command_t command = I2C_QUEUE.consumer_buffer->commands[I2C_QUEUE.consumer_buffer_cursor];
  I2C_QUEUE.interrupt_pending = true;

  switch (command.code) {

    case I2C_COMMAND_START:
      i2c_hw_send_start_condition();
      break;

    case I2C_COMMAND_SEND_DATA:
      i2c_hw_send_byte(command.data);
      break;

    case I2C_COMMAND_STOP:
    default:
      i2c_hw_send_stop_condition();
      I2C_QUEUE.interrupt_pending = false;
      break;
  }

  ++I2C_QUEUE.consumer_buffer_cursor;

  if (I2C_QUEUE.consumer_buffer_cursor == I2C_QUEUE.consumer_buffer->length) {
    I2C_QUEUE.consumer_buffer->busy = false;

    if (!I2C_QUEUE.producer_buffer->busy) {
      /* Producer is done, we can switch buffers now. */
      i2c_queue_switch_buffers();

      /* If previous buffer ended on stop condition, we have to manually process next command. */
      if (!I2C_QUEUE.interrupt_pending) {
        i2c_queue_process_command();
        I2C_QUEUE.interrupt_pending = true;
      }

      i2c_queue_fetch_commands();
    }
    else {
      /* Producer is not done, we have to wait for more commands.
       * Disable interrupt temporarily. */
      TWCR &= ~_BV(TWIE);

      if (ring_buffer_is_empty(&(I2C_QUEUE.tasks))) {
        I2C_QUEUE.is_idle = true;
      }
    }
  }
}


static void
i2c_queue_fetch_commands(void)
{
  assert_interrupts_disabled();

  bool consumer_done = false;

  do {
    if (ring_buffer_is_empty(&(I2C_QUEUE.tasks)))
      return;

    i2c_task_t *task = (i2c_task_t *) ring_buffer_get_first(&(I2C_QUEUE.tasks));

    assert(I2C_QUEUE.producer_buffer->length == 0);
    assert(I2C_QUEUE.producer_buffer->busy);

    NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
      task->callback(task->data);
    }

    assert(I2C_QUEUE.producer_buffer->length > 0);

    if (I2C_QUEUE.producer_buffer->commands[I2C_QUEUE.producer_buffer->length - 1].code == I2C_COMMAND_STOP)
    {
      i2c_queue_switch_tasks();
    }

    I2C_QUEUE.producer_buffer->busy = false;
    consumer_done = !(I2C_QUEUE.consumer_buffer->busy);

    if (consumer_done) {
      /* Other buffer was sent in full, we start processing next command and
       * proceed to produce more commands immediately. */
      i2c_queue_switch_buffers();
      i2c_queue_process_command();
    }
    /* Else: we finished before sending the other buffer, just exit */
  } while (consumer_done);
}


ISR(TWI_vect) {
  uint8_t i2c_status = TWSR & TW_STATUS_MASK;

  if (i2c_status > TW_MT_DATA_ACK || i2c_status == TW_MT_SLA_NACK) {
    fault(FAULT_I2C, i2c_status, NULL);
  }

  //~ if (I2C_QUEUE.consumer_buffer_cursor >= I2C_QUEUE.consumer_buffer->length) {
    //~ return;
  //~ }

  if (I2C_QUEUE.consumer_buffer_cursor >= I2C_QUEUE.consumer_buffer->length) {
    fault(FAULT_I2C, I2C_QUEUE.consumer_buffer->length, "buffer cursor out of range");
  }

  i2c_queue_process_command();
}

/* User API ----------------------------------------------------------------- */

void i2c_init(void)
{
  I2C_QUEUE.consumer_buffer = &(I2C_QUEUE.buffer[0]);
  I2C_QUEUE.consumer_buffer->busy = false;
  I2C_QUEUE.consumer_buffer->length = 0;
  I2C_QUEUE.consumer_buffer_cursor = 0;

  I2C_QUEUE.producer_buffer = &(I2C_QUEUE.buffer[1]);
  I2C_QUEUE.producer_buffer->busy = true;
  I2C_QUEUE.producer_buffer->length = 0;

  I2C_QUEUE.interrupt_pending = false;
  I2C_QUEUE.is_idle = true;

  ring_buffer_init(
    &(I2C_QUEUE.tasks),
    I2C_QUEUE.tasks_storage, /* storage */
    sizeof(i2c_task_t), /* element_size */
    sizeof(I2C_QUEUE.tasks_storage) / sizeof(i2c_task_t) /* elements_max */
  );

  i2c_hw_init();
}


bool i2c_is_idle(void)
{
  bool result = false;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    result = I2C_QUEUE.is_idle;
  }

  return result;
}



void
i2c_set_idle(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    I2C_QUEUE.is_idle = true;
  }
}


void
i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ring_buffer_assert_can_write_n_elements(&(I2C_QUEUE.tasks), 1);

    i2c_task_t *task = (i2c_task_t *) ring_buffer_append(&(I2C_QUEUE.tasks));

    task->address = address;
    task->callback = callback;
    task->data = data;

    if (I2C_QUEUE.is_idle) {
      I2C_QUEUE.is_idle = false;
      i2c_queue_fetch_commands();
    }
  }
}


static inline void
i2c_produce_command(i2c_command_code_t code, uint8_t data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    assert(I2C_QUEUE.producer_buffer->length < I2C_BUFFER_SIZE);
    assert(I2C_QUEUE.producer_buffer->busy);

    i2c_command_t *command = &(I2C_QUEUE.producer_buffer->commands[I2C_QUEUE.producer_buffer->length]);

    command->code = code;
    command->data = data;

    ++(I2C_QUEUE.producer_buffer->length);
  }
}


void
i2c_async_send_data(uint8_t data)
{
  i2c_produce_command(I2C_COMMAND_SEND_DATA, data);
}


void
i2c_async_send_start(void)
{
  ring_buffer_assert_can_read_n_elements(&(I2C_QUEUE.tasks), 1);

  i2c_task_t *task = (i2c_task_t *) ring_buffer_get_first(&(I2C_QUEUE.tasks));

  i2c_produce_command(I2C_COMMAND_START, 0);
  assert((task->address & 1) == 0);
  i2c_produce_command(I2C_COMMAND_SEND_DATA, task->address);
}


void
i2c_async_end_transmission(void)
{
  i2c_produce_command(I2C_COMMAND_STOP, 0);
}

/* end of User API ---------------------------------------------------------- */
