#include "view_channel.h"

#include "view.h"
#include "segdisp.h"
#include "view_data.h"

static uint8_t update_input (void *d) {
    struct view_data *data = (struct view_data *)d;
    if (data->buttons[1].high) {
        data->selected_ch = ((data->selected_ch - 1) + data->channel_count) % data->channel_count;
        return 1;
    }

    if (data->buttons[2].high) {
        data->selected_ch = (data->selected_ch + 1) % data->channel_count;
        return 1;
    }

    return 0;
}

static void update_display (void *d) {
    struct view_data *data = (struct view_data *)d;
    static char text[] = "---";
    text[0] = 'C';
    text[1] = 'H';
    text[2] = data->selected_ch + 1 + 48;

    segdisp_set_value(data->disp, text, 3);
}

void view_channel_init (struct view *v, void *d) {
    view_init(v, d, &update_input, &update_display);
}
