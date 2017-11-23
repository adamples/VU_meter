#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>


typedef struct ring_buffer_t_ {
  uint8_t element_size;
  uint8_t elements_max;
  uint8_t elements_n;
  void *read_pointer;
  void *write_pointer;
  void *storage;
} ring_buffer_t;


void ring_buffer_init(ring_buffer_t *buffer, void *storage, uint8_t element_size, uint8_t elements_max);
void ring_buffer_clear(ring_buffer_t *buffer);

bool ring_buffer_is_empty(ring_buffer_t *buffer);
uint8_t ring_buffer_get_size(ring_buffer_t *buffer);

void ring_buffer_push_byte(ring_buffer_t *buffer, uint8_t byte);
uint8_t ring_buffer_pop_byte(ring_buffer_t *buffer);

void ring_buffer_push(ring_buffer_t *buffer, void *data);
void ring_buffer_pop(ring_buffer_t *buffer, void *data);

void * ring_buffer_get_first(ring_buffer_t *buffer);

void * ring_buffer_append(ring_buffer_t *buffer);
void ring_buffer_discard(ring_buffer_t *buffer);


#define ring_buffer_assert_can_read_n_elements(buffer, n) \
  assert((buffer)->elements_n >= (n))

#define ring_buffer_assert_can_write_n_elements(buffer, n) \
  assert((buffer)->elements_n + (n) <= (buffer)->elements_max)


#endif /* RING_BUFFER_H */
