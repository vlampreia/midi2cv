#include "segdisp.h"

/* segment display pin-mapping for SC36-11SEKWA */
/**
 *    A
 *  F   B
 *    G
 *  E   C
 *    D
 *
 */
#define SEG_A (1 << 0)
#define SEG_B (1 << 1)
#define SEG_C (1 << 2)
#define SEG_D (1 << 3)
#define SEG_E (1 << 4)
#define SEG_F (1 << 5)
#define SEG_G (1 << 6)
#define SEG_DP (1 << 7)

/* font ASCII mapping*/
uint8_t font[125] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    SEG_G, // -
    SEG_DP, // .
    0x00,
    // (48)
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
    SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,
    SEG_B | SEG_C | SEG_F | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_G | SEG_F,
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // A (65)
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_F | SEG_E | SEG_D | SEG_C | SEG_G,
    SEG_G | SEG_D | SEG_E,
    SEG_B | SEG_E | SEG_D | SEG_C | SEG_G,
    SEG_F | SEG_E | SEG_D | SEG_A | SEG_G,
    SEG_A | SEG_G | SEG_E | SEG_F,
    SEG_A | SEG_F | SEG_E | SEG_D | SEG_C, // G
    SEG_C | SEG_E | SEG_F | SEG_G,
    0x00,
    0x00,
    0x00,
    SEG_F | SEG_E | SEG_D,
    0x00,
    SEG_E | SEG_G | SEG_C, // N
};


void segdisp_advance(struct segdisp *s, void (*spi_f)(uint8_t data)) {
    BIT_CLEAR(*(s->select_port), s->select_pins[s->current_display]);
    s->current_display = (s->current_display + 1) % s->displays;
    spi_f(s->values[s->current_display]);
    BIT_CLEAR(*(s->latch_port), s->latch_pin);
    BIT_SET(*(s->latch_port), s->latch_pin);
    BIT_SET(*(s->select_port), s->select_pins[s->current_display]);
}

void segdisp_set_value(struct segdisp *s, char *value, size_t n) {
    for (size_t i=0; i<n; ++i) {
        s->values[i] = font[*(value + i)];
    }
}

void segdisp_overlay_value(struct segdisp *s, char *value, size_t n) {
    for (size_t i=0; i<n; ++i) {
        s->values[i] |= font[*(value + i)];
    }
}
