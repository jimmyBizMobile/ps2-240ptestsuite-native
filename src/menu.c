#include "menu.h"
#include "patterns.h"
#include "video.h"
#include "font.h"
#include "sysinfo.h"
#include <gsKit.h>
#include <stdio.h>

// Menu entry table. Visibility and labels are computed per mode so the menu
// reflects the current region/scan:
//   - NTSC 240p shows "Grid full (240)" + "Grid (224)".
//   - All other modes show a single "Grid".
//   - Monoscope is shown disabled "(WIP)" in PAL.
//   - Region/Video are toggle items (CROSS changes them).
typedef struct {
    MenuItemId id;
    const char *label;   // static fallback label
    int is_system;       // 1 = blue group (Region/Video/Help/Credits)
} MenuEntry;

static MenuEntry g_entries[MENU_ITEM_COUNT] = {
    { MENU_GRID_FULL, "Grid full (240)", 0 },
    { MENU_GRID,      "Grid",        0 },
    { MENU_MONOSCOPE, "Monoscope",   0 },
    { MENU_PLUGE,     "Pluge",       0 },
    { MENU_COLORBARS, "Color bars",  0 },
    { MENU_SMPTE,     "SMPTE",       0 },
    { MENU_COLORBLEED,"Color Bleed", 0 },
    { MENU_REGION,    "Region",      1 },
    { MENU_VIDEO,     "Video",       1 },
    { MENU_HELP,      "Help",        1 },
    { MENU_CREDITS,   "Credits",     1 },
};

static int g_sel = 0;   // index into g_entries

// --- per-mode rules -------------------------------------------------------

// NTSC 240p (progressive, not PAL) is the only mode with the split grid.
static int ntsc_240p(void)
{
    return !video_is_pal() && VIDEO_IS_PROG(video_current_mode());
}

// Whether an entry is shown at all in the current mode.
static int item_visible(int idx)
{
    MenuItemId id = g_entries[idx].id;
    // MENU_GRID is always visible (label changes: "Grid" or "Grid (224)").
    // MENU_GRID_FULL only appears in NTSC 240p (the extra 240 option).
    if (id == MENU_GRID_FULL) return ntsc_240p();
    return 1;
}

// Whether an entry is shown but greyed/disabled (cannot be selected).
static int item_disabled(int idx)
{
    return g_entries[idx].id == MENU_MONOSCOPE && video_is_pal();
}

// Selectable = visible and not disabled.
static int item_selectable(int idx)
{
    return item_visible(idx) && !item_disabled(idx);
}

void menu_init(GSGLOBAL *gs)
{
    font_init(gs);
    if (!item_selectable(g_sel)) menu_validate_selection();
}

void menu_move_up(void)
{
    int guard = 0;
    do {
        g_sel = (g_sel - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
    } while (!item_selectable(g_sel) && guard++ < MENU_ITEM_COUNT);
}
void menu_move_down(void)
{
    int guard = 0;
    do {
        g_sel = (g_sel + 1) % MENU_ITEM_COUNT;
    } while (!item_selectable(g_sel) && guard++ < MENU_ITEM_COUNT);
}
MenuItemId menu_current(void) { return g_entries[g_sel].id; }

void menu_validate_selection(void)
{
    int guard = 0;
    while (!item_selectable(g_sel) && guard++ < MENU_ITEM_COUNT)
        g_sel = (g_sel + 1) % MENU_ITEM_COUNT;
}

// --- dynamic labels -------------------------------------------------------

static const char *video_label(void)
{
    switch (video_current_mode()) {
    case VIDEO_MODE_240P: return "Video [240p]";
    case VIDEO_MODE_480I: return "Video [480i]";
    case VIDEO_MODE_288P: return "Video [288p]";
    case VIDEO_MODE_576I: return "Video [576i]";
    default:              return "Video [240p]";
    }
}

static const char *region_label(void)
{
    return video_is_pal() ? "Region [PAL]" : "Region [NTSC]";
}

static const char *grid_label(void)
{
    // In NTSC 240p the grid is the 224 option (paired with "Grid full (240)").
    // In every other mode it is just "Grid".
    return ntsc_240p() ? "Grid (224)" : "Grid";
}

static const char *entry_label(int idx)
{
    switch (g_entries[idx].id) {
    case MENU_VIDEO:     return video_label();
    case MENU_REGION:    return region_label();
    case MENU_MONOSCOPE: return video_is_pal() ? "Monoscope (WIP)" : "Monoscope";
    case MENU_GRID:      return grid_label();
    case MENU_GRID_FULL: return "Grid full (240)";
    default:             return g_entries[idx].label;
    }
}

// --- draw -----------------------------------------------------------------

void menu_draw(GSGLOBAL *gs)
{
    patterns_draw_background(gs);

    int is_480i = !VIDEO_IS_PROG(video_current_mode());  // interlaced: 480i or 576i
    // PAL frames are taller, so the background's white band sits lower; push
    // the menu down by the extra height so the first item clears it.
    float pal_off = video_is_pal() ? (is_480i ? 32.0f : 16.0f) : 0.0f;
    float sc          = is_480i ? 0.7f  : 0.35f;
    float x           = is_480i ? 80.0f : 40.0f;
    float top         = (is_480i ? 90.0f : 48.0f) + pal_off;
    float line        = is_480i ? 26.0f : 11.5f;
    float group_gap   = is_480i ? 12.0f : 5.0f;   // space before the blue system group

    u64 white = GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);
    u64 blue  = GS_SETREG_RGBAQ(0x28, 0x41, 0x5A, 0x80, 0x00);
    u64 red   = GS_SETREG_RGBAQ(0x80, 0x20, 0x20, 0x80, 0x00);
    u64 gray  = GS_SETREG_RGBAQ(0x40, 0x40, 0x40, 0x80, 0x00);

    float y = top;
    int prev_system = 0;
    int first = 1;
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        if (!item_visible(i)) continue;

        if (g_entries[i].is_system && !prev_system && !first)
            y += group_gap;
        prev_system = g_entries[i].is_system;
        first = 0;

        u64 col;
        if (item_disabled(i))            col = gray;
        else if (i == g_sel)             col = red;
        else if (g_entries[i].is_system) col = blue;
        else                             col = white;

        font_print(gs, x, y, sc, col, entry_label(i));
        y += line;
    }

    // --- status bar ---
    int w = video_width();
    int h = video_height();
    float status_sc = is_480i ? 0.7f : 0.35f;
    float status_y  = h - (is_480i ? 66.0f : 33.0f);

    char resline[32];
    snprintf(resline, sizeof(resline), "%s %dx%d",
             video_is_pal() ? "PAL" : "NTSC", w, h);

    font_print(gs, x, status_y, status_sc, white, sysinfo_console_label());
    float rw = font_text_width(resline, status_sc);
    font_print(gs, w - rw - x, status_y, status_sc, white, resline);

    const char *outfmt = sysinfo_output_label();
    float ow = font_text_width(outfmt, status_sc);
    font_print(gs, w - ow - x, status_y + (is_480i ? 22.0f : 11.0f),
               status_sc, white, outfmt);
}