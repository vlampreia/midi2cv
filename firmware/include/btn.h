#ifndef MIDI2CV_BTN_H
#define MIDI2CV_BTN_H

#include <stdint.h>
#include <stdint.h>

struct btn {
    uint8_t  state;
    uint8_t  high;
    volatile uint8_t *port;
    uint8_t  pin;
};

void btn_init (struct btn *b, volatile uint8_t *port, uint8_t pin);

int btn_update_state (struct btn *b);

#endif
