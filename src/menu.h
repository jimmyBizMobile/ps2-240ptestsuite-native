#ifndef MENU_H
#define MENU_H

#include <gsKit.h>

// Menu item identifiers, returned by menu_current() on CROSS.
typedef enum {
    MENU_GRID = 0,        // normal grid (224 NTSC / mode height elsewhere)
    MENU_GRID_FULL,       // NTSC 240p only: full grid at 240 lines
    MENU_MONOSCOPE,
    MENU_PLUGE,
    MENU_COLORBARS,
    MENU_SMPTE,
    MENU_COLORBLEED,
    MENU_REGION,          // toggles NTSC <-> PAL
    MENU_VIDEO,           // toggles progressive <-> interlaced
    MENU_HELP,
    MENU_CREDITS,
    MENU_ITEM_COUNT
} MenuItemId;

void menu_init(GSGLOBAL *gs);
void menu_move_up(void);
void menu_move_down(void);
void menu_validate_selection(void);
MenuItemId menu_current(void);     // currently highlighted item

void menu_draw(GSGLOBAL *gs);

#endif
