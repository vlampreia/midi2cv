#include "cbuff.h"

#define CBUFF_MASK_IDX(index, size) ((index) & ((size) - 1))

void cbuff_init (struct cbuff *b, uint8_t *d, size_t s) {
    b->data = d;
    for (size_t i=0; i<s; ++i) {
        b->data[i] = 0;
    }

    b->size = s;
    b->read_idx = 0;
    b->write_idx = 1;
}

uint8_t cbuff_empty (struct cbuff *b) {
    return (b->read_idx == b->write_idx);
}

uint8_t cbuff_read (struct cbuff *b) {
    uint8_t d = b->data[CBUFF_MASK_IDX(b->read_idx, b->size)];
    b->read_idx++;
    return d;
}

// void cbuff_write (struct cbuff *b, uint8_t v) {
//     b->data[CBUFF_MASK_IDX(b->write_idx, b->size)] = v;
//     b->write_idx++;
// }
