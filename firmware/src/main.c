#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stddef.h>

#include "cbuff.h"
#include "mcp4802.h"
#include "cvchan.h"
#include "midi.h"
#include "segdisp.h"
#include "btn.h"
#include "view.h"
#include "view_data.h"
#include "view_channel.h"
#include "view_midi_channel.h"
#include "view_midi_note.h"
#include "helpers.h"

/* -------------------------------------------------------------------------- */

#define BUFFER_SIZE 32
#define MIDI_BUFFER_SIZE 64

#define DAC_LDAC_PORT PORTC
#define DAC_LDAC_PIN PINC3

#define CHANNEL_COUNT 8
#define DAC_COUNT (CHANNEL_COUNT > 1 ? CHANNEL_COUNT / 2 : 1) //mcp4802 holds two dacs each

#define BUTTON_COUNT 3

/* -------------------------------------------------------------------------- */

enum display_mode {
    disp_mode_channel,
    disp_mode_midi,
    disp_mode_note,
    disp_mode_debug
};

/* globals */
struct MCP4802  dacs[DAC_COUNT];
uint8_t         cbuff_buffers[CHANNEL_COUNT * BUFFER_SIZE];
struct cbuff    buffers[CHANNEL_COUNT];
struct cvchan   channels[CHANNEL_COUNT];
struct cbuff    midi_buffer = {0};
struct segdisp  disp = {0};
struct btn      buttons[BUTTON_COUNT];

static volatile uint8_t sig_proc_ana_buff = 0;
static volatile uint8_t update_display_trig = 0;
static volatile uint8_t update_midi_trig = 0;
static volatile uint8_t sample_input_trig = 0;
static volatile uint8_t process_input_trig  = 0;

struct view view_channel;
struct view view_midi_channel;
struct view view_midi_note;

enum display_mode   disp_mode;
uint8_t             disp_sent;
uint8_t             disp_dbg = 0;

/* non-volatile memory */
struct nv_channel {
    uint8_t midi_channel;
    uint8_t note;
};
static struct nv_channel EEMEM nv_channels[CHANNEL_COUNT];

/* functions */
static void spi_init (void);
static void spi_send (uint8_t data);
static void spi_send_uint16_t (uint16_t data);
static void buffer_advance (void);
static void init_timers (void);
static void setup_buffers (void);
static void setup_usart (void);
static void init_channels (void);
static void process_midi (uint8_t byte);
static void write_pulse (uint8_t peak_value, struct cbuff *b);
static void update_display (void);
static void store_settings (void);
static void load_settings (void);
static void config_gpio (void);
static uint8_t handle_ui_input (void);
static void setup_buttons(void);
static void update_inputs(void);
static void setup_display (void);
static void init_views (void);

/* -------------------------------------------------------------------------- */

void config_gpio (void) {
    DDRC = 0xFF;
    PORTC = 0x00;
    DDRD = (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (0 << DDD5) | (0 << DDD6) | (0 << DDD7);
    DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
    BIT_CLEAR(PORTC, PINC5);
    BIT_CLEAR(PORTC, PINC4);

    // internal inputs on 5,6,7 btn
    BIT_SET(PORTD, PIND5);
    BIT_SET(PORTD, PIND6);
    BIT_SET(PORTD, PIND7);
}

void init_channels (void) {
    for (size_t i=0; i<CHANNEL_COUNT; ++i) {
        MCP4802_init(&dacs[i/2], &DAC_LDAC_PORT, DAC_LDAC_PIN, &PORTC, PINC0);
        cbuff_init(&buffers[i], cbuff_buffers + i*BUFFER_SIZE, BUFFER_SIZE);
        cvchan_init(&channels[i], &buffers[i], &dacs[i/2], i % 2, 0, 0);
    }
}

void setup_usart (void) {
    /* set up usart for MIDI */
    /* frame format is ... */
    // baud rate
    uint16_t br = F_CPU / 16 / 31250 - 1;
    UBRR0H = (br >> 8);
    UBRR0L = (br & 0xFF);
    // frame format: 8 bit, 1 stop bit, 1 start bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // enable rx, rx isr
    UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
}

void buffer_advance (void) {
    uint8_t latch_pins = 0;

    for (int i=0; i<CHANNEL_COUNT; ++i) {
        if (cvchan_proc_buffer(&channels[i], &spi_send_uint16_t)) continue;
        latch_pins |= channels[i].dac->ldac_pin;
    }

    if (latch_pins) {
        MCP4802_latch_direct(DAC_LDAC_PORT, latch_pins);
    }
}

void spi_init (void) {
    // MOSI, SCK output, SS master
    PORTC |= (1 << PINB2);
    DDRB = (1 << DDB5) | (1 << DDB3) | (1 << DDB2);

    // Enable SPI, Master, clock rate fck/16
    // default write MSB-first
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);
}

void spi_send (uint8_t data) {
    SPDR = data;

    /* wait for spi to be free */
    while (!(SPSR & (1 << SPIF)) && (SPCR & (1 << MSTR))) {}

    if (!(SPCR & (1 << MSTR))) {
        // DDRB |= (1 << DDB2);
        SPCR |= (1 << MSTR);
    }

    // force clear
    SPSR;
}

void spi_send_uint16_t (uint16_t data) {
    spi_send((data >> 8));
    spi_send(data & 0x00FF);
}

void init_timers (void) {
    /**
     * timerFreq = Fclk_io / (2 * N * (1 + OCRnA))
     * max timerFreq = Fclk_io / 2 when OCRnA == 0
     * N == selected prescalar factor
     *
     * CS12 CS11 CS10 Desc
     *    0    0    0 no clk src
     *    0    0    1 clk_io/1    prescaling
     *    0    1    0 clk_io/8    prescaling
     *    0    1    1 clk_io/64   prescaling
     *    1    0    0 clk_io/256  prescaling
     *    1    0    1 clk_io/1024 prescaling
     *
     * 1ms = 1000hz = 20Mhz / (2*8*2500)
     */

    TCCR0A = (1 << WGM01); // set CTC mode, TOP = OCR0A
    TCCR0B = (1 << CS02) | (1 << CS00); // clk/1024
    OCR0A = 33;
    TIMSK0 = (1 << OCIE0A); // enable CTC compare A

}

/* ISR */

ISR (USART_RX_vect) {
    uint8_t data = UDR0;
    if (data >= 0xF0) {
        return; // filter non common msgs
    }
    cbuff_write(&midi_buffer, data);
}

ISR (TIMER0_COMPA_vect) {
    sig_proc_ana_buff++;
    update_display_trig++;
    update_midi_trig++;
    sample_input_trig++;
    process_input_trig++;
}

/* /ISR */

void setup_buffers (void) {
    static uint8_t midi_buffer_d[MIDI_BUFFER_SIZE];
    cbuff_init(&midi_buffer, midi_buffer_d, MIDI_BUFFER_SIZE);
}

void write_pulse (uint8_t peak_value, struct cbuff *b) {
    cbuff_write(b, peak_value);
}

void process_midi (uint8_t byte) {
    static struct midi_event event = { 0 };

    midi_parse_next(&event, byte);
    if (!event.has_d2) return;

    uint8_t value = 0;
    disp_sent = 0;

    for (int i=0; i<CHANNEL_COUNT; ++i) {
        if (cvchan_proc_midi(&channels[i], &event, &value)) {
            continue;
        }

        write_pulse(value, channels[i].buffer);
        if (value) disp_sent = 1;
    }

    update_display();
}

void update_display (void) {
    static char text[] = "---";

    switch (disp_mode) {
        case disp_mode_channel:
            view_update_display(&view_channel);
            break;

        case disp_mode_midi:
            view_update_display(&view_midi_channel);
            break;

        case disp_mode_note:
            view_update_display(&view_midi_note);
            break;

        case disp_mode_debug: {
            text[0] = ((disp_dbg / 100) % 10) + 48;
            text[1] = ((disp_dbg / 10) % 10) + 48;
            text[2] =  (disp_dbg % 10) + 48;
            segdisp_set_value(&disp, text, 3);

            break;
        }
    }

    if (disp_sent) segdisp_overlay_value(&disp, ".  ", 3);
}

uint8_t handle_ui_input (void) {
    static size_t disp_mode_idx = 0;
    static enum display_mode mode_order[] = {
        disp_mode_channel,
        disp_mode_midi,
        disp_mode_note,
    };

    if (buttons[0].high) {
        disp_mode = mode_order[(disp_mode_idx++) % 3];
        return 1;
    }

    switch (disp_mode) {
        case disp_mode_channel:
            return view_update_input(&view_channel);

        case disp_mode_midi:
            return view_update_input(&view_midi_channel);

        case disp_mode_note:
            return view_update_input(&view_midi_note);

        case disp_mode_debug:
            if (buttons[0].high) {
                disp_mode = mode_order[0];
                return 1;
            }
            if (buttons[1].high) {
                disp_dbg--;
                return 1;
            }
            if (buttons[2].high) {
                disp_dbg++;
                return 1;
            }

            return 0;

        default:
            return 0;
    }
}

//TODO: reenable wdt for eeprom write protection
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void get_mcusr (void) \
    __attribute__((naked)) \
    __attribute__((section(".init3")));
void get_mcusr (void) {
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

void load_settings (void) {
    struct nv_channel settings[CHANNEL_COUNT];
    eeprom_read_block(&settings, &nv_channels, sizeof(struct nv_channel) * CHANNEL_COUNT);

    for (size_t i=0; i<CHANNEL_COUNT; ++i) {
        channels[i].midi_channel = settings[i].midi_channel;
        channels[i].note = settings[i].note;
    }
}

void store_settings (void) {
    struct nv_channel settings[CHANNEL_COUNT];

    for (size_t i=0; i<CHANNEL_COUNT; ++i) {
        settings[i].midi_channel = channels[i].midi_channel;
        settings[i].note = channels[i].note;
    }

    eeprom_update_block(&settings, &nv_channels, sizeof(struct nv_channel) * CHANNEL_COUNT);
}

static void setup_buttons(void) {
    btn_init(&buttons[0], &PIND, PIND5);
    btn_init(&buttons[1], &PIND, PIND6);
    btn_init(&buttons[2], &PIND, PIND7);
}

static void update_inputs(void) {
    for (size_t i=0; i<BUTTON_COUNT; ++i) {
        if (btn_update_state(&buttons[i])) {
            process_input_trig = 100;
        }
    }
}

static void setup_display (void) {
    // TODO: refactor to module init func
    static uint8_t pins[] = { PIND2, PIND3, PIND4 };
    static uint8_t values[3];

    disp.displays = 3;
    disp.select_pins = pins;
    disp.current_display = 0;
    disp.values = values;
    disp.select_port = &PORTD;
    disp.latch_port = &PORTC;
    disp.latch_pin=PINC5;
}

static void init_views (void) {
    static struct view_data vd;
    vd.selected_ch = 0;
    vd.disp = &disp;
    vd.channels = channels;
    vd.buttons = buttons;
    vd.channel_count = CHANNEL_COUNT;

    view_channel_init(&view_channel, &vd);
    view_midi_channel_init(&view_midi_channel, &vd);
    view_midi_note_init(&view_midi_note, &vd);
}

/* -------------------------------------------------------------------------- */

int main (void) {
    cli();
    config_gpio();
    init_channels();
    setup_buffers();
    setup_buttons();
    init_timers();
    setup_usart();
    spi_init();
    sei();

    init_views();

    // shutdown dacs until first used
    for (size_t i=0; i<DAC_COUNT; ++i) {
        MCP4802_disable_dac_spi(&dacs[i], 0, &spi_send_uint16_t);
        MCP4802_disable_dac_spi(&dacs[i], 1, &spi_send_uint16_t);
    }

    load_settings();
    setup_display();

    // initialise ui
    disp_mode = disp_mode_debug;
    update_display();

    uint8_t user_change = 0;

    while (1) {
        sei(); // SREG:I does not always reset to 1. This should be investigated
        // uint8_t s = BIT_CHECK(SREG, 7); // check interrupt flag

        // read midi buffer
        if (update_midi_trig >= 1) {
            update_midi_trig = 0;

            if (!cbuff_empty(&midi_buffer)) {
                uint8_t d = cbuff_read(&midi_buffer);
                process_midi(d);
            }
        }

        // write analogue out
        if (sig_proc_ana_buff >= 2) {
            sig_proc_ana_buff = 0;

            buffer_advance();
        }

        // refresh display
        if (update_display_trig >= 1) {
            update_display_trig = 0;

            segdisp_advance(&disp, &spi_send);
        }

        // debounce ui input
        if (sample_input_trig >= 10) {
            sample_input_trig = 0;

            update_inputs();
        }

        // process user input
        if (process_input_trig >= 100) {
            process_input_trig = 0;

            if (!buttons[0].high && !buttons[1].high && !buttons[2].high) {
                if (user_change && eeprom_is_ready()) {
                    store_settings();
                    user_change = 0;
                }

                continue;
            }

            user_change = handle_ui_input();
            update_display();
        }
    }
}
