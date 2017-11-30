#include "i2c.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <util/twi.h>
#include "config.h"
#include "fault.h"
#include "assert.h"
#include "ring_buffer.h"


typedef struct i2c_task_t_ {
  i2c_callback_t callback;
  void *data;
  uint8_t address; /* TODO: remove */
} i2c_task_t;


typedef struct i2c_queue_t_ {
  i2c_task_t tasks_storage[I2C_QUEUE_SIZE];
  ring_buffer_t tasks;

  i2c_command_buffer_t buffer_a;
  i2c_command_buffer_t buffer_b;

  i2c_command_buffer_t *front_buffer;
  i2c_command_buffer_t *back_buffer;

  bool transmitter_active;
  bool pending_buffer_switch;
  uint8_t tasks_n;

  uint8_t front_buffer_cursor;
  i2c_command_t current_command;
} i2c_queue_t;


static i2c_queue_t I2C_QUEUE;


#define assert_interrupts_disabled() assert((SREG & _BV(SREG_I)) == 0)


/* Low level hardware operations -------------------------------------------- */

static inline void
i2c_hw_init(void)
{
  TWSR = 0x00;
  TWCR = 0x00;
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
}


static inline void
i2c_hw_wait(void)
{
  while (!(TWCR & _BV(TWINT)));
}


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
i2c_hw_disable_interrupt(void)
{
  /* Clear InterruptEnable bit, but don't write INT bit, so it doesn't get cleared */
  TWCR &= ~_BV(TWIE) & ~_BV(TWINT);
}

/* end of Low level hardware operations ------------------------------------- */

static void
i2c_queue_switch_buffers()
{
  assert(I2C_QUEUE.back_buffer->length > 0);
  assert(I2C_QUEUE.front_buffer_cursor == I2C_QUEUE.front_buffer->length);

  i2c_command_buffer_t *tmp = I2C_QUEUE.back_buffer;
  I2C_QUEUE.back_buffer = I2C_QUEUE.front_buffer;
  I2C_QUEUE.front_buffer = tmp;

  I2C_QUEUE.pending_buffer_switch = false;
  I2C_QUEUE.back_buffer->length = 0;
  I2C_QUEUE.front_buffer_cursor = 0;

  I2C_QUEUE.current_command = I2C_QUEUE.front_buffer->commands[0];
}


static void
i2c_queue_switch_tasks(void)
{
  assert_interrupts_disabled();
  ring_buffer_assert_can_read_n_elements(&(I2C_QUEUE.tasks), 1);

  ring_buffer_discard(&(I2C_QUEUE.tasks));
  --I2C_QUEUE.tasks_n;
}


static void i2c_queue_process_command(void);

static inline void i2c_queue_start_transmitter(void)
{
  assert(!(I2C_QUEUE.transmitter_active));
  assert(I2C_QUEUE.current_command.code != I2C_COMMAND_PENDING);

  I2C_QUEUE.transmitter_active = true;
  i2c_queue_process_command();
}


static void
i2c_queue_fetch_commands(void)
{
  assert_interrupts_disabled();
  assert(I2C_QUEUE.tasks_n > 0);
  assert(!(I2C_QUEUE.pending_buffer_switch));

  do {
    if (I2C_QUEUE.tasks_n == 0) {
      return;
    }

    i2c_task_t *task = (i2c_task_t *) ring_buffer_get_first(&(I2C_QUEUE.tasks));

    assert(I2C_QUEUE.back_buffer->length == 0);

    NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
      task->callback(I2C_QUEUE.back_buffer, task->data);
    }

    assert(I2C_QUEUE.back_buffer->length > 0);

    if (I2C_QUEUE.back_buffer->commands[I2C_QUEUE.back_buffer->length - 1].code == I2C_COMMAND_STOP)
    {
      i2c_queue_switch_tasks();
    }

    if (!I2C_QUEUE.transmitter_active) {
      /* Other buffer was sent in full, we start processing next command and
       * proceed to produce more commands immediately. */
      i2c_queue_switch_buffers();
      i2c_queue_start_transmitter();
    }
    else {
      /* We finished before sending the other buffer, schedule the switch */
      I2C_QUEUE.pending_buffer_switch = true;
      return;
    }
  } while (true);
}


static void
i2c_queue_process_command(void)
{
  assert_interrupts_disabled();
  assert(I2C_QUEUE.transmitter_active);

  assert(I2C_QUEUE.front_buffer->length > 0);
  assert(I2C_QUEUE.front_buffer_cursor <= I2C_QUEUE.front_buffer->length);

  switch (I2C_QUEUE.current_command.code) {

    case I2C_COMMAND_SEND_DATA:
      i2c_hw_send_byte(I2C_QUEUE.current_command.data);
      break;

    case I2C_COMMAND_START:
      i2c_hw_send_start_condition();
      break;

    case I2C_COMMAND_STOP:
      i2c_hw_send_stop_condition();
      I2C_QUEUE.transmitter_active = false;
      break;

    case I2C_COMMAND_PENDING:
      /* Run out of data... */
      if (I2C_QUEUE.pending_buffer_switch) {
        /* But more is waiting, so just switch and we're good to go */
        i2c_queue_switch_buffers();
        i2c_queue_process_command();

        if (I2C_QUEUE.tasks_n > 0) {
          i2c_queue_fetch_commands();
        }
      }
      else {
        /* No more data waiting - disable transmitter interrupt and exit */
        i2c_hw_disable_interrupt();
        I2C_QUEUE.transmitter_active = false;
      }

      return;

    default:
      fault(FAULT_I2C, I2C_QUEUE.current_command.code, "Invalid command");
      break;
  }

  ++I2C_QUEUE.front_buffer_cursor;

  if (I2C_QUEUE.front_buffer_cursor == I2C_QUEUE.front_buffer->length) {
    if (I2C_QUEUE.pending_buffer_switch) {
      /* Producer is done, we can switch buffers now. */
      i2c_queue_switch_buffers();

      /* If previous buffer ended on stop condition, we have to manually process next command. */
      if (!I2C_QUEUE.transmitter_active) {
        i2c_queue_start_transmitter();
      }

      if (I2C_QUEUE.tasks_n > 0) {
        i2c_queue_fetch_commands();
      }
    }
    else {
      I2C_QUEUE.current_command.code = I2C_COMMAND_PENDING;
    }
  }
  else {
    I2C_QUEUE.current_command = I2C_QUEUE.front_buffer->commands[I2C_QUEUE.front_buffer_cursor];
    assert(I2C_QUEUE.transmitter_active);
  }
}


ISR(TWI_vect) {
  uint8_t i2c_status = TWSR & TW_STATUS_MASK;

  if (i2c_status > TW_MT_DATA_ACK || i2c_status == TW_MT_SLA_NACK) {
    fault(FAULT_I2C, i2c_status, "Error status");
  }

  i2c_queue_process_command();
}

/* User API ----------------------------------------------------------------- */

void i2c_init(void)
{
  I2C_QUEUE.front_buffer = &(I2C_QUEUE.buffer_a);
  I2C_QUEUE.front_buffer->length = 0;
  I2C_QUEUE.front_buffer_cursor = 0;

  I2C_QUEUE.back_buffer = &(I2C_QUEUE.buffer_b);
  I2C_QUEUE.back_buffer->length = 0;

  I2C_QUEUE.pending_buffer_switch = false;
  I2C_QUEUE.transmitter_active = false;
  I2C_QUEUE.tasks_n = 0;

  I2C_QUEUE.current_command.code = I2C_COMMAND_PENDING;

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
    result = !(I2C_QUEUE.transmitter_active) && (I2C_QUEUE.tasks_n == 0);
  }

  return result;
}


void
i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ring_buffer_assert_can_write_n_elements(&(I2C_QUEUE.tasks), 1);

    i2c_task_t *task = (i2c_task_t *) ring_buffer_append(&(I2C_QUEUE.tasks));
    bool is_idle = i2c_is_idle();

    task->address = address;
    task->callback = callback;
    task->data = data;

    ++I2C_QUEUE.tasks_n;

    if (is_idle) {
      i2c_queue_fetch_commands();
    }
  }
}

static inline void
i2c_produce_command(i2c_command_code_t code, uint8_t data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    assert(I2C_QUEUE.back_buffer->length < I2C_BUFFER_SIZE);

    i2c_command_t *command = &(I2C_QUEUE.back_buffer->commands[I2C_QUEUE.back_buffer->length]);

    command->code = code;
    command->data = data;

    ++(I2C_QUEUE.back_buffer->length);
  }
}


extern inline void
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
