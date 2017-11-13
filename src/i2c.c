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
  I2C_COMMAND_REPEATED_START = 0x01,
  I2C_COMMAND_SEND_DATA = 0x02,
  I2C_COMMAND_STOP = 0x03
} i2c_command_code_t;


typedef struct i2c_task_t_ {
  i2c_callback_t callback;
  void *data;
  uint8_t address;
} i2c_task_t;


typedef struct i2c_queue_t_ {
  uint8_t buffer_storage[I2C_BUFFER_SIZE];
  ring_buffer_t buffer;
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

/* end of Low level hardware operations ------------------------------------- */

static void
i2c_queue_fetch_commands(void)
{
  assert_interrupts_disabled();

  i2c_task_t *task = (i2c_task_t *) ring_buffer_get_first(&(I2C_QUEUE.tasks));
  //~ i2c_task_t *task = (i2c_task_t *) I2C_QUEUE.tasks.read_pointer;

  task->callback(task->data);
}


static void
i2c_queue_start_task(void)
{
  assert_interrupts_disabled();

  i2c_hw_send_start_condition();

  ring_buffer_clear(&(I2C_QUEUE.buffer));
  i2c_queue_fetch_commands();
}


static void
i2c_queue_end_task(void)
{
  assert_interrupts_disabled();

  i2c_hw_send_stop_condition();
  assert(ring_buffer_get_size(&(I2C_QUEUE.buffer)) == 0);

  ring_buffer_discard(&(I2C_QUEUE.tasks));

  uint8_t tasks_n = I2C_QUEUE.tasks.elements_n; // ring_buffer_get_size(&(I2C_QUEUE.tasks));
  PORTD = ~(0xff << tasks_n);

  if (tasks_n == 0) {
    i2c_hw_go_idle();
  }
  else {
    /* Start new transmission */
    i2c_queue_start_task();
  }
}


static void
i2c_queue_send_sla(void)
{
  assert_interrupts_disabled();

  i2c_task_t *task = (i2c_task_t *) ring_buffer_get_first(&(I2C_QUEUE.tasks));
  //~ i2c_task_t *task = (i2c_task_t *) I2C_QUEUE.tasks.read_pointer;

  i2c_hw_send_byte(task->address);
}


static void
i2c_queue_process_command(void)
{
  assert_interrupts_disabled();

  uint8_t command_code = ring_buffer_pop_byte(&(I2C_QUEUE.buffer));

  switch (command_code) {

    case I2C_COMMAND_REPEATED_START:
      i2c_hw_send_start_condition();
      break;

    case I2C_COMMAND_SEND_DATA:
      i2c_hw_send_byte(ring_buffer_pop_byte(&(I2C_QUEUE.buffer)));
      break;

    case I2C_COMMAND_STOP:
    default:
      i2c_queue_end_task();
      break;
  }

  if (command_code != I2C_COMMAND_STOP && ring_buffer_get_size(&(I2C_QUEUE.buffer)) == 0) {
    i2c_queue_fetch_commands();
  }
}


ISR(TWI_vect) {
  uint8_t i2c_status = TWSR & TW_STATUS_MASK;

  switch (i2c_status) {

    case TW_START:
    case TW_REP_START:
      i2c_queue_send_sla();
      break;

    case TW_MT_SLA_ACK:
    case TW_MT_DATA_ACK:
      i2c_queue_process_command();
      break;

    case TW_MT_SLA_NACK:
    case TW_MT_DATA_NACK:
    default:
      fault(FAULT_I2C, i2c_status, NULL);
      break;
  }
}

/* User API ----------------------------------------------------------------- */

void i2c_init(void)
{
  ring_buffer_init(
    &(I2C_QUEUE.buffer),
    I2C_QUEUE.buffer_storage, /* storage */
    1, /* element_size */
    sizeof(I2C_QUEUE.buffer_storage) /* elements_max */
  );

  ring_buffer_init(
    &(I2C_QUEUE.tasks),
    I2C_QUEUE.tasks_storage, /* storage */
    sizeof(i2c_task_t), /* element_size */
    sizeof(I2C_QUEUE.tasks_storage) / sizeof(i2c_task_t) /* elements_max */
  );

  TWSR = 0x00;
  TWCR = 0x00;
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

  DDRD = 0xff;
}


void
i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    i2c_task_t *task = (i2c_task_t *) ring_buffer_append(&(I2C_QUEUE.tasks));

    task->address = address;
    task->callback = callback;
    task->data = data;

    uint8_t tasks_n = I2C_QUEUE.tasks.elements_n; // ring_buffer_get_size(&(I2C_QUEUE.tasks));

    PORTD = ~(0xff << tasks_n);

    if (tasks_n == 1) {
      /* There were no tasks prior to this one, we must start transmission */
      i2c_queue_start_task();
    }
  }
}


void
i2c_async_send_data(uint8_t data)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ring_buffer_push_byte(&(I2C_QUEUE.buffer), I2C_COMMAND_SEND_DATA);
    ring_buffer_push_byte(&(I2C_QUEUE.buffer), data);
  }
}


void
i2c_async_send_repeated_start(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ring_buffer_push_byte(&(I2C_QUEUE.buffer), I2C_COMMAND_REPEATED_START);
  }
}


void
i2c_async_end_transmission(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ring_buffer_push_byte(&(I2C_QUEUE.buffer), I2C_COMMAND_STOP);
  }
}

/* end of User API ---------------------------------------------------------- */
