// PS2 240p Test Suite (native) - Wii-style menu.
//
// Controls:
//   UP/DOWN  - move selection
//   CROSS    - activate item
//   CIRCLE   - return to menu (from a pattern/info scene)
//   TRIANGLE - toggle 240p/480i (any scene)
//   SELECT   - quit to browser

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
    int is_480i = (video_current_mode() == VIDEO_MODE_480I);
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
    "Monoscope   - overall calibration",
    "Pluge       - black level (brightness)",
    "Color bars  - color / saturation",
    "SMPTE       - color check, SQUARE toggles 75/100",
    "Color bleed - stripes show color smearing",
    "TRIANGLE toggles 240p / 480i",
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

    GSGLOBAL *gs = video_init(VIDEO_MODE_240P);
    pad_init();
    reload_all(gs);

    while (1) {
        pad_update();

        if (pad_pressed(PAD_SELECT)) break;

        if (pad_pressed(PAD_TRIANGLE)) {
            video_toggle_mode(gs);
            reload_all(gs);
        }
        if (g_scene == SCENE_SMPTE && pad_pressed(PAD_SQUARE)){
            patterns_smpte_toggle();
        }


        if (g_scene == SCENE_MENU) {
            if (pad_pressed(PAD_UP))   menu_move_up();
            if (pad_pressed(PAD_DOWN)) menu_move_down();
            if (pad_pressed(PAD_CROSS)) {
                switch (menu_current()) {
                case MENU_GRID:       g_scene = SCENE_GRID;       break;
                case MENU_MONOSCOPE:  g_scene = SCENE_MONOSCOPE;  break;
                case MENU_PLUGE:      g_scene = SCENE_PLUGE;      break;
                case MENU_COLORBARS:  g_scene = SCENE_COLORBARS;  break;
                case MENU_SMPTE:      g_scene = SCENE_SMPTE;      break;
                case MENU_COLORBLEED: g_scene = SCENE_COLORBLEED; break;
                case MENU_VIDEO:
                    video_toggle_mode(gs);
                    reload_all(gs);
                    break;
                case MENU_HELP:    g_scene = SCENE_HELP;    break;
                case MENU_CREDITS: g_scene = SCENE_CREDITS; break;
                default: break;
                }
            }
        } else {
            if (pad_pressed(PAD_CIRCLE)) g_scene = SCENE_MENU;
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