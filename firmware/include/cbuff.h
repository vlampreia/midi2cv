#ifndef MIDI2CV_CBUFF_H
#define MIDI2CV_CBUFF_H

#include <stddef.h>
#include <stdint.h>

#define CBUFF_MASK_IDX(index, size) ((index) & ((size) - 1))

struct cbuff {
    volatile uint8_t *data;
    volatile size_t size;
    volatile size_t read_idx;
    volatile size_t write_idx;
};

void cbuff_init (struct cbuff *b, uint8_t *d, size_t s);

uint8_t cbuff_empty (struct cbuff *b);

uint8_t cbuff_read (struct cbuff *b);

// void cbuff_write (struct cbuff *b, uint8_t v);
static inline void cbuff_write (struct cbuff *b, uint8_t v) {
    b->data[CBUFF_MASK_IDX(b->write_idx, b->size)] = v;
    b->write_idx++;
}

#endif
