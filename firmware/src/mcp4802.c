#include "mcp4802.h"
#include "helpers.h"

void MCP4802_init (
    struct MCP4802 *d,
    volatile uint8_t *ldac_port,
    uint8_t ldac_pin,
    volatile uint8_t *cs_port,
    uint8_t cs_pin
) {
    d->ldac_port = ldac_port;
    d->ldac_pin = ldac_pin;
    d->cs_port = cs_port;
    d->cs_pin = cs_pin;
}

void MCP4802_latch (struct MCP4802 *d) {
    BIT_CLEAR(*(d->ldac_port), d->ldac_pin);
    BIT_SET(*(d->ldac_port), d->ldac_pin);
}

void MCP4802_latch_direct (volatile uint8_t port, uint8_t pins) {
    BIT_CLEAR(port, pins);
    BIT_SET(port, pins);
}

void MCP4802_send_spi (
    struct MCP4802 *d,
    int dac,
    uint8_t value,
    void (*spi_f)(uint16_t data)
) {
    uint16_t cmd = (1 << MCP4802_P_SHDN) |
        (1 << MCP4802_P_GA) |
        (dac << MCP4802_P_DAC) |
        (value << 4);

    MCP4802_send_raw_spi(d, cmd, spi_f);
}

void MCP4802_send_raw_spi (
    struct MCP4802 *d,
    uint16_t cmd,
    void (*spi_f)(uint16_t data)
) {
    BIT_CLEAR(*(d->cs_port), d->cs_pin);
    (*spi_f)(cmd);
    BIT_SET(*(d->cs_port), d->cs_pin);
}

void MCP4802_disable_dac_spi (
    struct MCP4802 *d,
    int dac,
    void (*spi_f)(uint16_t data)
) {
    uint16_t cmd = (dac << MCP4802_P_DAC);
    MCP4802_send_raw_spi(d, cmd, spi_f);
}
