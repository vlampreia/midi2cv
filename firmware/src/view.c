#include "view.h"

void view_init (
    struct view *v,
    void *data,
    uint8_t (*update_input)     (void *d),
    void    (*update_display)   (void *d)
) {
    v->data = data;
    v->update_input = update_input;
    v->update_display = update_display;
}

uint8_t view_update_input (struct view *v) {
    return v->update_input(v->data);
}

void view_update_display (struct view *v) {
    v->update_display(v->data);
}
