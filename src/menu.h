#ifndef MENU_H
#define MENU_H

#include <gsKit.h>

// Menu item identifiers, returned by menu_selected_action() on CROSS.
typedef enum {
    MENU_GRID = 0,
    MENU_MONOSCOPE,
    MENU_PLUGE,
    MENU_COLORBARS,
    MENU_VIDEO,
    MENU_HELP,
    MENU_CREDITS,
    MENU_ITEM_COUNT
} MenuItemId;

void menu_init(GSGLOBAL *gs);
void menu_move_up(void);
void menu_move_down(void);
MenuItemId menu_current(void);     // currently highlighted item

void menu_draw(GSGLOBAL *gs);

#endif
