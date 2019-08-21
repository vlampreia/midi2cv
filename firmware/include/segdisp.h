#ifndef MIDI2CV_SEGDISP_H
#define MIDI2CV_SEGDISP_H

#include <stdint.h>
#include <stddef.h>
#include "helpers.h"

struct segdisp {
    size_t displays;
    volatile uint8_t *select_port;
    uint8_t *select_pins;
    volatile uint8_t *latch_port;
    uint8_t latch_pin;
    size_t current_display;
    uint8_t *values;
};

void segdisp_advance(struct segdisp *s, void (*spi_f)(uint8_t data));
void segdisp_set_value(struct segdisp *s, char *value, size_t n);
void segdisp_overlay_value(struct segdisp *s, char *value, size_t n);

#endif
