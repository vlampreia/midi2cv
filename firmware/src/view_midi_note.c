#include "view_midi_note.h"

#include "view.h"
#include "segdisp.h"
#include "view_data.h"

static void update_display (void *d) {
    struct view_data *data = (struct view_data *)d;
    static char text[] = "---";
    struct cvchan *ch = &(data->channels[data->selected_ch]);
    uint8_t note = ch->note % 12;
    uint8_t oct = ch->note / 12;

    uint8_t note_map[] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

    text[0] = ' ';
    text[1] = note_map[note];

    if (ch->note == 255) { //omni
        text[0] = 'N';
        text[1] = 'A';
        text[2] = 'L';
    } else {
        if (oct == 0) {
            text[0] = '-';
            text[2] = 1 + 48;
        } else {
            text[2] = oct + 48 - 1;
        }
    }

    segdisp_set_value(data->disp, text, 3);

    if (note > 0 && (text[1] == note_map[note-1])) {
        segdisp_overlay_value(data->disp, " . ", 3);
    }
}

static uint8_t update_input (void *d) {
    struct view_data *data = (struct view_data *)d;
    struct cvchan *ch = &(data->channels[data->selected_ch]);

    if (data->buttons[1].high) {
        if (data->buttons[2].high) {
            ch->note = 255;
            return 1;
        }

        ch->note = ((ch->note - 1) + 127) % 127;

        return 1;
    }

    if (data->buttons[2].high) {
        ch->note = (ch->note + 1) % 127;

        return 1;
    }

    return 0;
}

void view_midi_note_init (struct view *v, void *d) {
    view_init(v, d, &update_input, &update_display);
}
