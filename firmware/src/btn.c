#include "btn.h"

#define EDGE_LEADING 0x0F
#define EDGE_FALLING 0xF0

void btn_init(struct btn *b, volatile uint8_t *port, uint8_t pin) {
    b->state = 0;
    b->high = 0;
    b->port = port;
    b->pin = pin;
}

int btn_update_state(struct btn *b) {
    b->state = (b->state << 1) | !(*(b->port) & (1 << b->pin));

    if (b->state == EDGE_LEADING) {
        b->high = 1;
        return 1;
    } else if (b->state == EDGE_FALLING){
        b->high = 0;
        return 1;
    }

    return 0;
}
