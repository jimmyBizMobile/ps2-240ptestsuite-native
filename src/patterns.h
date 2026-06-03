#ifndef PATTERNS_H
#define PATTERNS_H

#include <gsKit.h>

void patterns_load(GSGLOBAL *gs);

void patterns_draw_grid(GSGLOBAL *gs);
void patterns_draw_monoscope(GSGLOBAL *gs);
void patterns_draw_background(GSGLOBAL *gs);

void patterns_draw_pluge(GSGLOBAL *gs);
void patterns_draw_colorbars(GSGLOBAL *gs);

void patterns_draw_smpte(GSGLOBAL *gs);
void patterns_smpte_toggle(void);
int patterns_smpte_is_100(void);
void patterns_draw_colorbleed(GSGLOBAL *gs);

#endif