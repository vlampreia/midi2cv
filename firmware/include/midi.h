#ifndef MIDI2CV_MIDI_H
#define MIDI2CV_MIDI_H

#include <stdint.h>

#define MIDI_S_MASK 0x80
#define MIDI_S_NOTEOFF 0
#define MIDI_S_NOTEON 1

struct midi_event {
    uint8_t channel;
    uint8_t cmd;
    uint8_t d1;
    uint8_t d2;
    uint8_t has_d1;
    uint8_t has_d2;
};

void midi_parse_next (struct midi_event *state, uint8_t byte);

#endif
