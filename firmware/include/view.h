#ifndef MIDI2CV_VIEW_H
#define MIDI2CV_VIEW_H

#include <stdint.h>

struct view {
    uint8_t (*update_input)     (void *d);
    void    (*update_display)   (void *d);

    void *data;
};

void view_init (
    struct view *v,
    void *data,
    uint8_t (*update_input)     (void *d),
    void    (*update_display)   (void *d)
);

uint8_t view_update_input (struct view *v);

void view_update_display (struct view *v);

#endif
