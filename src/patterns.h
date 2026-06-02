#ifndef PATTERNS_H
#define PATTERNS_H

#include <gsKit.h>

void patterns_load(GSGLOBAL *gs);

void patterns_draw_grid(GSGLOBAL *gs);
void patterns_draw_monoscope(GSGLOBAL *gs);
void patterns_draw_background(GSGLOBAL *gs);

// Placeholders reusing the grid texture until real assets are provided.
void patterns_draw_pluge(GSGLOBAL *gs);
void patterns_draw_colorbars(GSGLOBAL *gs);

void patterns_shift_left(void);
void patterns_shift_right(void);
float patterns_shift_value(void);

#endif