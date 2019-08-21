/**
 *
 *  MCP4802 voltage scaling:
 *      v_out = (2.048v * D) / 2^n * G
 *          Dn = input code
 *          G = Gain selection: !GA(0) -> 2x, !GA(1) -> 1x
 *
 *  MCP4802 write instruction:
 *       bit 15 ..                              .. bit 8
 *      | (!A)/B | x  | !GA | !SHDN | D7 | D6 | D5 | D4 |
 *      | D3     | D2 | D1  | D0    | x  | x  | x  | x  |
 *       bit 7 ..                               .. bit 0
 *
 *      (!A)/B    = DAC selection bit
 *      !GA   = gain selection bit: 0 -> 2x, 1 -> 1x
 *      !SHDN   = DAC disable bit
 *      D7...D0 = data bits
 *      x       = ignored
 *
 *  MCP4802 control pins:
 *      !CS   = Chip select (low = write enable)
 *      SCK   = Serial data clock
 *      SDI   = Serial data
 *      !LDAC = Data latch pulse (low = latch output)
 *
 *  MCP4802 pinout:
 *      1 - vdd
 *      2 - !CS /0
 *      3 - SCK /1
 *      4 - SDI /2
 *      5 - Vout_a
 *      6 - Vss
 *      7 - Vout_b
 *      8 - !LDAC
 *
 */

#ifndef MIDI2CV_MCP4802_H
#define MIDI2CV_MCP4802_H

#include <stdint.h>

#define MCP4802_P_SHDN 12
#define MCP4802_P_GA 13
#define MCP4802_P_DAC 15

struct MCP4802 {
    volatile uint8_t *ldac_port;
    uint8_t ldac_pin;
    volatile uint8_t *cs_port;
    uint8_t cs_pin;
};

void MCP4802_init (
    struct MCP4802 *d,
    volatile uint8_t *ldac_port,
    uint8_t ldac_pin,
    volatile uint8_t *cs_port,
    uint8_t cs_pin
);

void MCP4802_latch (struct MCP4802 *d);
void MCP4802_latch_direct (volatile uint8_t port, uint8_t pins);

void MCP4802_send_spi (
    struct MCP4802 *d,
    int dac,
    uint8_t value,
    void (*spi_f)(uint16_t data)
);

void MCP4802_send_raw_spi (
    struct MCP4802 *d,
    uint16_t cmd,
    void (*spi_f)(uint16_t data)
);

void MCP4802_disable_dac_spi (
    struct MCP4802 *d,
    int dac,
    void (*spi_f)(uint16_t data)
);

#endif
