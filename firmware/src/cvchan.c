#include "cvchan.h"
#include "cbuff.h"
#include "mcp4802.h"
#include "midi.h"

void cvchan_init (
    struct cvchan *c,
    struct cbuff *b,
    struct MCP4802 *d,
    uint8_t dac_n,
    uint8_t midi_ch,
    uint8_t note
) {
    c->buffer = b;
    c->dac = d;
    c->dac_n = dac_n;
    c->midi_channel = midi_ch;
    c->note = note;
}

uint8_t cvchan_proc_midi (struct cvchan *c, struct midi_event *e, uint8_t *v) {
    if (e->cmd != MIDI_S_NOTEON) return 1;
    if (e->channel != c->midi_channel) return 2;
    if (e->d1 != c->note) return 3;

    *v = e->d2 * 2;

    return CVCHAN_PM_VALID;
}

uint8_t cvchan_proc_buffer (struct cvchan *c, void (*spi_f)(uint16_t data)) {
    if (cbuff_empty(c->buffer)) return CVCHAN_PB_NOOP;

    uint8_t d = cbuff_read(c->buffer);
    MCP4802_send_spi(c->dac, c->dac_n, d, spi_f);
    // MCP4802_latch(c->dac);
    return CVCHAN_PB_SENT;
}
