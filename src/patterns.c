#include "patterns.h"
#include "assets_data.h"
#include "video.h"
#include "font.h"
#include "monoscope_rects.h"
#include <stdint.h>
#include <string.h>
#include <kernel.h>

// Background is still a texture; grid and monoscope are procedural.
typedef struct { GSTEXTURE tex; int real_w, real_h; } Pattern;
static Pattern g_bg;

static void load_one(GSGLOBAL *gs, Pattern *p,
                     int buf_w, int buf_h, int real_w, int real_h,
                     const uint32_t *pixels)
{
    GSTEXTURE *tex = &p->tex;
    memset(tex, 0, sizeof(*tex));
    tex->Width = buf_w; tex->Height = buf_h;
    tex->PSM = GS_PSM_CT32; tex->Mem = (u32 *)pixels;
    tex->Filter = GS_FILTER_NEAREST; tex->Delayed = 0;
    tex->TBW = (buf_w + 63) / 64;
    p->real_w = real_w; p->real_h = real_h;
    FlushCache(0);
    tex->Vram = gsKit_vram_alloc(gs,
        gsKit_texture_size(tex->Width, tex->Height, tex->PSM),
        GSKIT_ALLOC_USERBUFFER);
    if (tex->Vram != GSKIT_ALLOC_ERROR)
        gsKit_texture_upload(gs, tex);
}

void patterns_load(GSGLOBAL *gs)
{
    gsKit_vram_clear(gs);
    load_one(gs, &g_bg, ASSET_BACKGROUND_W, ASSET_BACKGROUND_H,
             ASSET_BACKGROUND_REAL_W, ASSET_BACKGROUND_REAL_H, asset_background_pixels);
}

static void blit_fullscreen(GSGLOBAL *gs, Pattern *p)
{
    int sw = video_width(), sh = video_height();
    u64 mod = GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);
    gsKit_prim_sprite_texture(gs, &p->tex,
        0.0f, 0.0f, 0.0f, 0.0f,
        (float)sw, (float)sh,
        (float)p->real_w, (float)p->real_h, 1, mod);
}

// --- Artemio grid, drawn procedurally with solid primitives. ---
//
// Pixel-exact reproduction of the 320x224 reference (verified 0 diff).
// The screen is a uniform grid of 16x16 cells (20 x 14 at 240p). Each cell
// boundary has a 2px line; the cell center has a 2x2 dot. Outer-ring cells
// are RED (border), interior cells WHITE. At a red/white boundary, each of
// the two line pixels is colored by the cell it faces (outer pixel = outer
// cell, inner pixel = inner cell).
//
// Drawn with gsKit_prim_sprite (solid primitives) which - unlike textured
// sprites in this gsKit - rasterize the full width/height with no edge or
// strip artifacts. Scales by an integer factor in 480i.
static void px(GSGLOBAL *gs, int x, int y, int w, int h, int s, u64 col)
{
    // Draw a w x h block of cells-space pixels at scale s, as one sprite.
    gsKit_prim_sprite(gs, (float)(x*s), (float)(y*s),
                          (float)((x+w)*s), (float)((y+h)*s), 2, col);
}

void patterns_draw_grid(GSGLOBAL *gs)
{
    int sw = video_width();
    int s  = sw / 320;              // 1 at 240p, 2 at 480i
    if (s < 1) s = 1;

    const int CELL = 16;
    const int NCOLS = 320 / CELL;   // 20
    const int NROWS = 224 / CELL;   // 14
    const int W = 320, H = 224;

    u64 black = GS_SETREG_RGBAQ(0, 0, 0, 0x80, 0);
    u64 red   = GS_SETREG_RGBAQ(0xFF, 0, 0, 0x80, 0);
    u64 white = GS_SETREG_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0);

    gsKit_clear(gs, black);

    #define CELLCOL(gx,gy) (((gx)==0 || (gy)==0 || (gx)==NCOLS-1 || (gy)==NROWS-1) ? red : white)

    // Vertical grid lines: at each boundary m, 2px line at columns m*16-1
    // (owned by left cell) and m*16 (owned by right cell), per row segment.
    for (int m = 0; m <= NCOLS; m++) {
        int xL = m*CELL - 1, xR = m*CELL;
        for (int gy = 0; gy < NROWS; gy++) {
            int y0 = gy*CELL;
            u64 lc = (m-1 >= 0)     ? CELLCOL(m-1, gy) : red;
            u64 rc = (m   < NCOLS)  ? CELLCOL(m,   gy) : red;
            if (xL >= 0) px(gs, xL, y0, 1, CELL, s, lc);
            if (xR < W)  px(gs, xR, y0, 1, CELL, s, rc);
        }
    }
    // Horizontal grid lines.
    for (int m = 0; m <= NROWS; m++) {
        int yT = m*CELL - 1, yB = m*CELL;
        for (int gx = 0; gx < NCOLS; gx++) {
            int x0 = gx*CELL;
            u64 tc = (m-1 >= 0)    ? CELLCOL(gx, m-1) : red;
            u64 bc = (m   < NROWS) ? CELLCOL(gx, m)   : red;
            if (yT >= 0) px(gs, x0, yT, CELL, 1, s, tc);
            if (yB < H)  px(gs, x0, yB, CELL, 1, s, bc);
        }
    }
    // Center dots (2x2) per cell.
    for (int gy = 0; gy < NROWS; gy++)
        for (int gx = 0; gx < NCOLS; gx++)
            px(gs, gx*CELL + 7, gy*CELL + 7, 2, 2, s, CELLCOL(gx, gy));

    #undef CELLCOL
}

void patterns_draw_monoscope(GSGLOBAL *gs)
{
    // Drawn from auto-generated rectangles (pixel-exact with the source PNG)
    // using solid primitives - the full-width-correct path, same as the
    // grid. No texture, so no edge clipping, no shift, no padding needed.
    int sw = video_width();
    int s  = sw / MONOSCOPE_SRC_W;          // 1 at 240p, 2 at 480i
    if (s < 1) s = 1;

    u64 red   = GS_SETREG_RGBAQ(0xFF, 0,    0,    0x80, 0);
    u64 white = GS_SETREG_RGBAQ(0xFF, 0xFF, 0xFF, 0x80, 0);

    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0x80, 0));
    for (int i = 0; i < MONOSCOPE_RECT_COUNT; i++) {
        const Rect *r = &monoscope_rects[i];
        u64 col = (r->col == 1) ? red : white;
        gsKit_prim_sprite(gs,
            (float)(r->x * s), (float)(r->y * s),
            (float)((r->x + r->w) * s), (float)((r->y + r->h) * s),
            2, col);
    }
}
void patterns_draw_background(GSGLOBAL *gs) { blit_fullscreen(gs, &g_bg); }


// PLUGE - black-level / brightness calibration (also white level on CRTs).
// Drawn with solid primitives; pixel layout matches the reference, scaled
// for the mode. The PS2 outputs RGB/component full-range, so black is true
// 0 and the just-above-black side bars (16, 32) are the right references.
// White is 255 (PS2 max) to match the colorbars.
//   center grayscale stack: x 120..199 (80 wide), y 32..191, four 40px
//     bands - white, light gray, mid gray, dark gray.
//   side bars (y 32..191): outer bars at x40/x264 (value 32, lighter),
//     inner bars at x72/x232 (value 16, darker).
// Calibration: lower Brightness until the side bars disappear, then raise
// until they're just barely visible.
void patterns_draw_pluge(GSGLOBAL *gs)
{
    int sw = video_width();
    int s  = sw / 320;
    if (s < 1) s = 1;

    u64 c_white = GS_SETREG_RGBAQ(255, 255, 255, 0x80, 0);  // PS2 max white
    u64 c_lgray = GS_SETREG_RGBAQ(176, 176, 176, 0x80, 0);
    u64 c_mgray = GS_SETREG_RGBAQ(72,  72,  72,  0x80, 0);
    u64 c_dgray = GS_SETREG_RGBAQ(48,  48,  48,  0x80, 0);
    u64 c_outer = GS_SETREG_RGBAQ(32,  32,  32,  0x80, 0);  // outer side bars
    u64 c_inner = GS_SETREG_RGBAQ(16,  16,  16,  0x80, 0);  // inner side bars

    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0x80, 0));

    #define R(x,y,w,h,col) gsKit_prim_sprite(gs, \
        (float)((x)*s), (float)((y)*s), \
        (float)(((x)+(w))*s), (float)(((y)+(h))*s), 2, (col))

    // center grayscale stack (4 bands of 40px)
    R(120, 32,  80, 40, c_white);
    R(120, 72,  80, 40, c_lgray);
    R(120, 112, 80, 40, c_mgray);
    R(120, 152, 80, 40, c_dgray);
    // side bars (160px tall)
    R(40,  32, 16, 160, c_outer);   // left outer
    R(72,  32, 16, 160, c_inner);   // left inner
    R(232, 32, 16, 160, c_inner);   // right inner
    R(264, 32, 16, 160, c_outer);   // right outer

    #undef R
}



// Color bars (PS2 / 8-bit version) - per-channel intensity ramps based on
// the CPS-2 color adjust pattern, for setting black/white/colour levels.
// The PS2 outputs full 8-bit colour, so this shows the complete 0x00..0xFF
// range at regular hex intervals (column n = n * 0x11), 16 columns labelled
// 0..F. Drawn with solid primitives; scales for 240p/480i.
// Calibration: set black with the PLUGE first (the "1" column should be
// barely visible). Then raise each Contrast / colour-level control until the
// top two steps (E and F) stop separating, and lower until just distinct.
void patterns_draw_colorbars(GSGLOBAL *gs)
{
    int sw = video_width();
    int s  = sw / 320;
    if (s < 1) s = 1;

    const int NCOL = 16;        // 0x0 .. 0xF
    const int X0 = 32, STEPW = 16, BARH = 32;     // 16*16 = 256px wide
    const int yR = 40, yG = 80, yB = 120, yW = 160;

    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0x80, 0));

    #define R(x,y,w,h,col) gsKit_prim_sprite(gs, \
        (float)((x)*s), (float)((y)*s), \
        (float)(((x)+(w))*s), (float)(((y)+(h))*s), 2, (col))

    for (int i = 0; i < NCOL; i++) {
        int v = i * 0x11;            // 0x00, 0x11, 0x22, ... 0xFF
        int x = X0 + i * STEPW;
        R(x, yR, STEPW, BARH, GS_SETREG_RGBAQ(v, 0, 0, 0x80, 0)); // red
        R(x, yG, STEPW, BARH, GS_SETREG_RGBAQ(0, v, 0, 0x80, 0)); // green
        R(x, yB, STEPW, BARH, GS_SETREG_RGBAQ(0, 0, v, 0x80, 0)); // blue
        R(x, yW, STEPW, BARH, GS_SETREG_RGBAQ(v, v, v, 0x80, 0)); // white/gray
    }

    #undef R

    // Channel labels.
    u64 white = GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0);
    float fsc = (s >= 2) ? 1.0f : 0.5f;
    font_print(gs, 2.0f * s, (yR + 11) * s, fsc, white, "RED");
    font_print(gs, 2.0f * s, (yG + 11) * s, fsc, white, "GREEN");
    font_print(gs, 2.0f * s, (yB + 11) * s, fsc, white, "BLUE");
    font_print(gs, 2.0f * s, (yW + 11) * s, fsc, white, "WHITE");

    // Optional hex column header 0..F across the top of the bars.
    const char *hex = "0123456789ABCDEF";
    char ch[2] = {0,0};
    for (int i = 0; i < NCOL; i++) {
        ch[0] = hex[i];
        font_print(gs, (X0 + i*STEPW + 4) * s, (yR - 12) * s, fsc, white, ch);
    }
}