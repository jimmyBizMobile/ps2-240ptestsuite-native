// Bitmap-atlas font renderer. Draws strings by blitting glyph
// sub-rectangles from an embedded atlas texture. Works with any ASCII
// string at runtime; no per-label assets needed.

#ifndef FONT_H
#define FONT_H

#include <gsKit.h>

// Upload the atlas texture to VRAM. Call after video init and after any
// mode switch (VRAM is reset).
void font_init(GSGLOBAL *gs);

// Draw a string. (x,y) is the top-left. scale multiplies glyph size
// (1.0 = native, 2.0 = double for 480i). color is a GS_SETREG_RGBAQ
// modulation color (use white 0x80 to show atlas as-is, or a tint).
void font_print(GSGLOBAL *gs, float x, float y, float scale, u64 color,
                const char *text);

// Measure the pixel width a string would occupy at the given scale,
// useful for centering.
float font_text_width(const char *text, float scale);

#endif