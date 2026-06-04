// PS2 240p Test Suite (native) - Wii-style menu.
//
// Controls:
//   UP/DOWN  - move selection (menu)
//   CROSS    - activate item; on Region/Video items, toggles them
//   CIRCLE   - return to menu (from a pattern/info scene)
//   SQUARE   - SMPTE 75%/100% (on the SMPTE screen)
//   SELECT   - quit to browser
// Mode (progressive/interlaced) and Region (NTSC/PAL) are changed only via
// their menu items - never from within a pattern scene.

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <gsKit.h>
#include <libpad.h>

#include "video.h"
#include "pad.h"
#include "patterns.h"
#include "menu.h"
#include "font.h"
#include "sysinfo.h"

typedef enum {
    SCENE_MENU,
    SCENE_GRID,
    SCENE_MONOSCOPE,
    SCENE_PLUGE,
    SCENE_COLORBARS,
    SCENE_SMPTE,
    SCENE_COLORBLEED,
    SCENE_HELP,
    SCENE_CREDITS,
} Scene;

static Scene g_scene = SCENE_MENU;

static void reload_all(GSGLOBAL *gs)
{
    patterns_load(gs);
    menu_init(gs);
}

static void draw_info(GSGLOBAL *gs, const char *const *lines, int n)
{
    patterns_draw_background(gs);
    int is_480i = !VIDEO_IS_PROG(video_current_mode());  // interlaced: 480i or 576i
    float sc   = is_480i ? 0.8f : 0.4f;
    float x    = is_480i ? 60.0f : 30.0f;
    float top  = is_480i ? 80.0f : 40.0f;
    float line = is_480i ? 24.0f : 12.0f;
    u64 white  = GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);
    for (int i = 0; i < n; i++)
        font_print(gs, x, top + i*line, sc, white, lines[i]);
}

static const char *help_lines[] = {
    "HELP",
    "",
    "Grid        - geometry / convergence",
    "Monoscope   - overall calibration (NTSC)",
    "Pluge       - black level (brightness)",
    "Color bars  - color / saturation",
    "SMPTE       - color check, SQUARE toggles 75/100",
    "Color bleed - stripes show color smearing",
    "Region item - toggle NTSC / PAL",
    "Video item  - toggle progressive / interlaced",
    "CIRCLE returns to this menu",
    "SELECT quits to browser",
};

static const char *credits_lines[] = {
    "CREDITS",
    "",
    "Patterns by Artemio Urbina",
    "Monoscope by Keith Raney",
    "",
    "PS2 native port",
    "built with PS2SDK + gsKit",
    "",
    "Developed by Rokusho",
};

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);

    // Start in the progressive mode matching the console's region:
    // 288p on a PAL console, 240p on NTSC.
    int initial_mode = sysinfo_is_pal() ? VIDEO_MODE_288P : VIDEO_MODE_240P;
    GSGLOBAL *gs = video_init(initial_mode);
    pad_init();
    reload_all(gs);

    while (1) {
        pad_update();

        if (pad_pressed(PAD_SELECT)) break;

        if (g_scene == SCENE_MENU) {
            if (pad_pressed(PAD_UP))   menu_move_up();
            if (pad_pressed(PAD_DOWN)) menu_move_down();
            if (pad_pressed(PAD_CROSS)) {
                switch (menu_current()) {
                case MENU_GRID:
                    // Normal grid (224 in NTSC 240p, mode height elsewhere).
                    video_set_ntsc_full_grid(gs, 0);
                    g_scene = SCENE_GRID;
                    break;
                case MENU_GRID_FULL:
                    // NTSC 240p full grid: switch framebuffer to 240 lines.
                    video_set_ntsc_full_grid(gs, 1);
                    reload_all(gs);
                    g_scene = SCENE_GRID;
                    break;
                case MENU_MONOSCOPE:
                    if (!video_is_pal()) g_scene = SCENE_MONOSCOPE;  // NTSC only
                    break;
                case MENU_PLUGE:      g_scene = SCENE_PLUGE;      break;
                case MENU_COLORBARS:  g_scene = SCENE_COLORBARS;  break;
                case MENU_SMPTE:      g_scene = SCENE_SMPTE;      break;
                case MENU_COLORBLEED: g_scene = SCENE_COLORBLEED; break;
                case MENU_REGION:
                    video_toggle_region(gs);
                    menu_validate_selection();
                    reload_all(gs);
                    break;
                case MENU_VIDEO:
                    video_toggle_mode(gs);
                    menu_validate_selection();
                    reload_all(gs);
                    break;
                case MENU_HELP:    g_scene = SCENE_HELP;    break;
                case MENU_CREDITS: g_scene = SCENE_CREDITS; break;
                default: break;
                }
            }
        } else {
            // In a pattern/info scene only CIRCLE (back) and, on SMPTE,
            // SQUARE (75/100%) do anything. No mode/region changes here.
            if (g_scene == SCENE_SMPTE && pad_pressed(PAD_SQUARE))
                patterns_smpte_toggle();
            if (pad_pressed(PAD_CIRCLE)) {
                // Leaving the grid: restore the normal 224 framebuffer if the
                // full-grid 240 view was active.
                if (g_scene == SCENE_GRID && video_ntsc_full_grid()) {
                    video_set_ntsc_full_grid(gs, 0);
                    reload_all(gs);
                }
                g_scene = SCENE_MENU;
            }
        }

        video_clear_black(gs);

        switch (g_scene) {
        case SCENE_MENU:      menu_draw(gs);                break;
        case SCENE_GRID:      patterns_draw_grid(gs);       break;
        case SCENE_MONOSCOPE: patterns_draw_monoscope(gs);  break;
        case SCENE_PLUGE:     patterns_draw_pluge(gs);      break;
        case SCENE_COLORBARS: patterns_draw_colorbars(gs);  break;
        case SCENE_SMPTE:     patterns_draw_smpte(gs);      break;
        case SCENE_COLORBLEED:patterns_draw_colorbleed(gs); break;
        case SCENE_HELP:
            draw_info(gs, help_lines, sizeof(help_lines)/sizeof(help_lines[0]));
            break;
        case SCENE_CREDITS:
            draw_info(gs, credits_lines, sizeof(credits_lines)/sizeof(credits_lines[0]));
            break;
        }

        video_flip(gs);
    }

    return 0;
}