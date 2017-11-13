#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>


typedef struct ring_buffer_t_ {
  uint8_t capacity;
  uint8_t size;
  uint8_t *read_pointer;
  uint8_t *write_pointer;
  uint8_t *data;
} ring_buffer_t;


void ring_buffer_init(ring_buffer_t *buffer, uint8_t *data, uint8_t capacity);
void ring_buffer_clear(ring_buffer_t *buffer);

uint8_t ring_buffer_get_size(ring_buffer_t *buffer);

void ring_buffer_push_byte(ring_buffer_t *buffer, uint8_t byte);
uint8_t ring_buffer_pop_byte(ring_buffer_t *buffer);

void ring_buffer_push(ring_buffer_t *buffer, void *data, uint8_t size);
void ring_buffer_pop(ring_buffer_t *buffer, void *data, uint8_t size);

#endif /* RING_BUFFER_H */
