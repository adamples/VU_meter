#include "ring_buffer.h"
#include <stdlib.h>
#include "assert.h"


void
ring_buffer_init(ring_buffer_t *buffer, uint8_t *data, uint8_t capacity)
{
  buffer->capacity = capacity;
  buffer->data = data;
  buffer->read_pointer = buffer->data;
  ring_buffer_clear(buffer);
}


void
ring_buffer_clear(ring_buffer_t *buffer)
{
  buffer->size = 0;
  buffer->write_pointer = buffer->read_pointer;
}


uint8_t
ring_buffer_get_size(ring_buffer_t *buffer)
{
  return buffer->size;
}


static void
ring_buffer_advance_pointer(ring_buffer_t *buffer, uint8_t **pointer)
{
  ++(*pointer);
  if (*pointer == buffer->data + buffer->capacity) {
    *pointer = buffer->data;
  }
}


void
ring_buffer_push_byte(ring_buffer_t *buffer, uint8_t byte)
{
  assert(buffer->size < buffer->capacity);

  (*buffer->write_pointer) = byte;
  ring_buffer_advance_pointer(buffer, &(buffer->write_pointer));
  ++buffer->size;
}


uint8_t
ring_buffer_pop_byte(ring_buffer_t *buffer)
{
  uint8_t result = (*buffer->read_pointer);

  assert(buffer->size > 0);

  ring_buffer_advance_pointer(buffer, &(buffer->read_pointer));

  --buffer->size;
  return result;
}


void ring_buffer_push(ring_buffer_t *buffer, void *data, uint8_t size)
{
  for (; size != 0; --size, ++data) {
    ring_buffer_push_byte(buffer, *(uint8_t *) data);
  }
}


void ring_buffer_pop(ring_buffer_t *buffer, void *data, uint8_t size)
{
  for (; size != 0; --size, ++data) {
    *(uint8_t *) data = ring_buffer_pop_byte(buffer);
  }
}
