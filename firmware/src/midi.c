/**
 * MIDI Spec
 *
 * frame format:
 *  1 start bit
 *  8 data bits
 *  1 stop bit
 *
 * @ 31250 Baud
 *
 * data format:
 *  Status: 1tttnnnn
 *  Data:   0xxxxxxx
 *
 *  t = message type
 *  n = midi channel
 *  x = message data
 *
 * 10010001
 * 10001001
 */

#include "midi.h"

void midi_parse_next (struct midi_event *state, uint8_t byte) {
    if (byte & MIDI_S_MASK) {
        state->channel = (byte & 0x0F);
        state->cmd = ((byte >> 4) & 0x07);
        state->d1 = 0;
        state->d2 = 0;
        state->has_d1 = 0;
        state->has_d2 = 0;

        return;
    }

    if (!state->has_d1) {
        state->d1 = byte;
        state->has_d1 = 1;

        return;
    }

    state->d2 = byte;
    state->has_d2 = 1;
    state->has_d1 = 0; // permit running status
}
