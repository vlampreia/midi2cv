#ifndef MIDI2CV_VIEW_DATA_H
#define MIDI2CV_VIEW_DATA_H

#include "cvchan.h"
#include "btn.h"
#include "segdisp.h"

struct view_data {
    uint8_t selected_ch;
    struct btn *buttons;
    struct segdisp *disp;
    struct cvchan *channels;
    size_t channel_count;
};

#endif
