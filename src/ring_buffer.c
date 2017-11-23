#include "ring_buffer.h"
#include <string.h>
#include "assert.h"


#define assert_is_byte_buffer(buffer) \
  assert((buffer)->element_size == 1)


void
ring_buffer_init(ring_buffer_t *buffer, void *storage, uint8_t element_size, uint8_t elements_max)
{
  buffer->element_size = element_size;
  buffer->elements_max = elements_max;
  buffer->storage = storage;
  buffer->read_pointer = buffer->storage;

  ring_buffer_clear(buffer);
}


void
ring_buffer_clear(ring_buffer_t *buffer)
{
  buffer->elements_n = 0;
  buffer->write_pointer = buffer->read_pointer;
}


bool
ring_buffer_is_empty(ring_buffer_t *buffer)
{
  return buffer->write_pointer == buffer->read_pointer;
}


uint8_t
ring_buffer_get_size(ring_buffer_t *buffer)
{
  return buffer->elements_n;
}


static void
ring_buffer_advance_pointer(ring_buffer_t *buffer, void **pointer)
{
  (*pointer) += buffer->element_size;
  if (*pointer == buffer->storage + buffer->elements_max * buffer->element_size) {
    *pointer = buffer->storage;
  }
}


void
ring_buffer_push_byte(ring_buffer_t *buffer, uint8_t byte)
{
  assert_is_byte_buffer(buffer);
  ring_buffer_assert_can_write_n_elements(buffer, 1);

  uint8_t *byte_write_pointer = (uint8_t *) (buffer->write_pointer);

  *(byte_write_pointer) = byte;
  ring_buffer_advance_pointer(buffer, &(buffer->write_pointer));
  ++buffer->elements_n;
}


uint8_t
ring_buffer_pop_byte(ring_buffer_t *buffer)
{
  uint8_t *byte_read_pointer = (uint8_t *) (buffer->read_pointer);
  uint8_t result = *(byte_read_pointer);

  assert_is_byte_buffer(buffer);
  ring_buffer_assert_can_read_n_elements(buffer, 1);

  ring_buffer_advance_pointer(buffer, &(buffer->read_pointer));

  --buffer->elements_n;
  return result;
}


void
ring_buffer_push(ring_buffer_t *buffer, void *element)
{
  ring_buffer_assert_can_write_n_elements(buffer, 1);

  memcpy(buffer->write_pointer, element, buffer->element_size);
  ring_buffer_advance_pointer(buffer, &(buffer->write_pointer));
  ++buffer->elements_n;
}


void
ring_buffer_pop(ring_buffer_t *buffer, void *data)
{
  ring_buffer_assert_can_read_n_elements(buffer, 1);

  memcpy(data, buffer->read_pointer, buffer->element_size);
  ring_buffer_advance_pointer(buffer, &(buffer->read_pointer));
  --buffer->elements_n;
}


void
ring_buffer_push_bytes(ring_buffer_t *buffer, uint8_t *bytes, uint8_t n)
{
  assert_is_byte_buffer(buffer);
  ring_buffer_assert_can_read_n_elements(buffer, n);

  uint8_t elements_wo_folding = buffer->elements_max - (buffer->write_pointer - buffer->storage);

  if (n <= elements_wo_folding) {
    memcpy(buffer->write_pointer, bytes, n);
    buffer->write_pointer += n;
  }
  else {
    memcpy(buffer->write_pointer, bytes, elements_wo_folding);
    memcpy(buffer->storage, bytes + elements_wo_folding, n - elements_wo_folding);
    buffer->write_pointer = buffer->storage + (n - elements_wo_folding);
  }

  ring_buffer_advance_pointer(buffer, &(buffer->write_pointer));
  buffer->elements_n += n;
}


inline void *
ring_buffer_get_first(ring_buffer_t *buffer)
{
  ring_buffer_assert_can_read_n_elements(buffer, 1);

  return buffer->read_pointer;
}


inline void *
ring_buffer_get_last(ring_buffer_t *buffer)
{
  ring_buffer_assert_can_read_n_elements(buffer, 1);

  return buffer->write_pointer;
}


void *
ring_buffer_append(ring_buffer_t *buffer)
{
  ring_buffer_assert_can_write_n_elements(buffer, 1);

  void *result = buffer->write_pointer;
  ring_buffer_advance_pointer(buffer, &(buffer->write_pointer));
  ++buffer->elements_n;
  return result;
}


void
ring_buffer_discard(ring_buffer_t *buffer)
{
  ring_buffer_assert_can_read_n_elements(buffer, 1);

  ring_buffer_advance_pointer(buffer, &(buffer->read_pointer));
  --buffer->elements_n;
}
