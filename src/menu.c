#include "menu.h"
#include "patterns.h"
#include "video.h"
#include "font.h"
#include "sysinfo.h"
#include <gsKit.h>
#include <stdio.h>

// Menu items. The "system" group (Video/Help/Credits) is drawn blue and
// sits below a gap after the "test pattern" group.
typedef struct {
    MenuItemId id;
    const char *label;
    int is_system;   // 1 = blue group, 0 = white group
} MenuEntry;

static MenuEntry g_entries[MENU_ITEM_COUNT] = {
    { MENU_GRID,      "Grid",       0 },
    { MENU_MONOSCOPE, "Monoscope",  0 },
    { MENU_PLUGE,     "Pluge",      0 },
    { MENU_COLORBARS, "Color bars", 0 },
    { MENU_SMPTE,     "SMPTE",      0 },
    { MENU_COLORBLEED,"Color Bleed",0 },
    { MENU_VIDEO,     "Video [240p]", 1 },
    { MENU_HELP,      "Help",       1 },
    { MENU_CREDITS,   "Credits",    1 },
};

static int g_sel = 0;

void menu_init(GSGLOBAL *gs)
{
    font_init(gs);
}

void menu_move_up(void)   { g_sel = (g_sel - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT; }
void menu_move_down(void) { g_sel = (g_sel + 1) % MENU_ITEM_COUNT; }
MenuItemId menu_current(void) { return g_entries[g_sel].id; }

// Keep the "Video [240p]" / "Video [480i]" label in sync with the mode.
static const char *video_label(void)
{
    return (video_current_mode() == VIDEO_MODE_240P)
        ? "Video [240p]" : "Video [480i]";
}

void menu_draw(GSGLOBAL *gs)
{
    patterns_draw_background(gs);

    int is_480i = (video_current_mode() == VIDEO_MODE_480I);
    float sc          = is_480i ? 0.7f  : 0.35f;
    float x           = is_480i ? 80.0f : 40.0f;
    float top         = is_480i ? 90.0f  : 45.0f;
    float line        = is_480i ? 26.0f  : 13.0f;
    float group_gap   = is_480i ? 28.0f  : 14.0f;  // extra space before system group

    // Colors (GS modulation of the white atlas glyphs)
    u64 white = GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);
    u64 blue = GS_SETREG_RGBAQ(0x28, 0x41, 0x5A, 0x80, 0x00);
    u64 red   = GS_SETREG_RGBAQ(0x80, 0x20, 0x20, 0x80, 0x00);

    float y = top;
    int prev_system = 0;
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        // Insert the gap when transitioning from test-pattern to system group
        if (g_entries[i].is_system && !prev_system) {
            y += group_gap;
        }
        prev_system = g_entries[i].is_system;

        u64 col;
        if (i == g_sel)               col = red;
        else if (g_entries[i].is_system) col = blue;
        else                          col = white;

        const char *label = (g_entries[i].id == MENU_VIDEO)
            ? video_label() : g_entries[i].label;

        font_print(gs, x, y, sc, col, label);
        y += line;
    }

    // --- Status bar at the bottom ---
    int w = video_width();
    int h = video_height();
    float status_sc = is_480i ? 0.7f : 0.35f;
    float status_y  = h - (is_480i ? 66.0f : 33.0f);

    char resline[32];
    snprintf(resline, sizeof(resline), "NTSC %dx%d", w, h);

    // Console label read from the PS2's own settings.
    font_print(gs, x, status_y, status_sc, white, sysinfo_console_label());
    // Right-align the resolution text
    float rw = font_text_width(resline, status_sc);
    font_print(gs, w - rw - x, status_y, status_sc, white, resline);

    // Output type (RGB / YPbPr) read from the PS2's Component Video setting.
    const char *outfmt = sysinfo_output_label();
    float ow = font_text_width(outfmt, status_sc);
    font_print(gs, w - ow - x, status_y + (is_480i ? 22.0f : 11.0f),
               status_sc, white, outfmt);
}
