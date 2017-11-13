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
  struct i2c_task_t_ *next;
} i2c_task_t;


typedef struct i2c_queue_t_ {
  i2c_task_t *current;
  i2c_task_t *last;
  uint8_t buffer_storage[I2C_BUFFER_SIZE];
  ring_buffer_t buffer;
} i2c_queue_t;


static i2c_queue_t I2C_QUEUE = { 0 };
static volatile uint16_t QUEUE_LEN = 0;


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
  assert((SREG & _BV(SREG_I)) == 0);

  i2c_task_t *task = I2C_QUEUE.current;

  task->callback(task->data);
}


static void
i2c_queue_start_task(void)
{
  assert((SREG & _BV(SREG_I)) == 0);

  i2c_hw_send_start_condition();

  ring_buffer_clear(&(I2C_QUEUE.buffer));
  i2c_queue_fetch_commands();
}


static void
i2c_queue_end_task(void)
{
  assert((SREG & _BV(SREG_I)) == 0);

  i2c_hw_send_stop_condition();
  assert(ring_buffer_get_size(&(I2C_QUEUE.buffer)) == 0);

  i2c_task_t *old_current = I2C_QUEUE.current;
  I2C_QUEUE.current = old_current->next;

  --QUEUE_LEN;
  PORTD = ~(0xff << QUEUE_LEN);

  if (I2C_QUEUE.current == NULL) {
    I2C_QUEUE.last = NULL;
    i2c_hw_go_idle();
  }
  else {
    /* Start new transmission */
    i2c_queue_start_task();
  }

  free(old_current);
}


static void
i2c_queue_push_task(i2c_task_t *task)
{
  assert((SREG & _BV(SREG_I)) == 0);

  ++QUEUE_LEN;
  PORTD = ~(0xff << QUEUE_LEN);

  if (I2C_QUEUE.current == NULL) {
    I2C_QUEUE.current = task;

    i2c_queue_start_task();
  }
  else {
    I2C_QUEUE.last->next = task;
  }

  I2C_QUEUE.last = task;
}


static void
i2c_queue_send_sla(void)
{
  assert((SREG & _BV(SREG_I)) == 0);

  i2c_task_t *task = I2C_QUEUE.current;

  i2c_hw_send_byte(task->address);
}


static void
i2c_queue_process_command(void)
{
  assert((SREG & _BV(SREG_I)) == 0);

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
  I2C_QUEUE.current = NULL;
  I2C_QUEUE.last = NULL;
  ring_buffer_init(&(I2C_QUEUE.buffer), I2C_QUEUE.buffer_storage, sizeof(I2C_QUEUE.buffer_storage));

  TWSR = 0x00;
  TWCR = 0x00;
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;

  DDRD = 0xff;
}


void
i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data)
{
  i2c_task_t *task = (i2c_task_t *) malloc(sizeof(i2c_task_t));

  task->address = address;
  task->callback = callback;
  task->data = data;
  task->next = NULL;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    i2c_queue_push_task(task);
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
