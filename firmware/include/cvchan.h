#ifndef MIDI2CV_CVCHAN_H
#define MIDI2CV_CVCHAN_H

#include <stdint.h>
#include "midi.h"

#define CVCHAN_PB_SENT 0
#define CVCHAN_PB_NOOP 1

#define CVCHAN_PM_VALID 0
#define CVCHAN_PM_IGNORE 1

struct cvchan {
    struct cbuff *buffer;
    struct MCP4802 *dac;
    uint8_t dac_n;
    uint8_t midi_channel;
    uint8_t note;
};

void cvchan_init (
    struct cvchan *c,
    struct cbuff *b,
    struct MCP4802 *d,
    uint8_t dac_n,
    uint8_t midi_ch,
    uint8_t note
);

uint8_t cvchan_proc_midi (struct cvchan *c, struct midi_event *e, uint8_t *v);

uint8_t cvchan_proc_buffer (struct cvchan *c, void (*spi_f)(uint16_t data));

#endif
