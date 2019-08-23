#include "view_midi_channel.h"

#include "view.h"
#include "segdisp.h"
#include "view_data.h"

static void update_display (void *d) {
    struct view_data *data = (struct view_data *)d;
    static char text[] = "---";
    struct cvchan *ch = &(data->channels[data->selected_ch]);
    text[0] = ' ';

    if (ch->midi_channel == 255) { // omni
        text[0] = 'C';
        text[1] = 'A';
        text[2] = 'L';
    } else {
        uint8_t disp_channel = ch->midi_channel + 1;

        text[1] = ((disp_channel / 10) % 10) + 48;
        text[2] =  (disp_channel % 10) + 48;
    }

    segdisp_set_value(data->disp, text, 3);
}

static uint8_t update_input (void *d) {
    struct view_data *data = (struct view_data *)d;
    struct cvchan *ch = &(data->channels[data->selected_ch]);

    if (data->buttons[1].high) {
        if (data->buttons[2].high) {
            ch->midi_channel = 255;
            return 1;
        }

        ch->midi_channel = ((ch->midi_channel - 1) + 16) % 16;
        return 1;
    }

    if (data->buttons[2].high) {
        ch->midi_channel = (ch->midi_channel + 1) % 16;
        return 1;
    }

    return 0;
}

void view_midi_channel_init (struct view *v, void *d) {
    view_init(v, d, &update_input, &update_display);
}
