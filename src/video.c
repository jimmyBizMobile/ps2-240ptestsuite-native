#include "video.h"
#include <gsKit.h>
#include <dmaKit.h>

// Real framebuffer dimensions (safe-area heights; the mode NAMES are
// 240p/480i/288p/576i, but we render at these conventional heights).
#define FB_240P_W  320
#define FB_240P_H  224
#define FB_240P_FULL_H 240    // NTSC progressive "full grid" height (15 rows)
#define FB_480I_W  640
#define FB_480I_H  448
#define FB_288P_W  320
#define FB_288P_H  256
#define FB_576I_W  640
#define FB_576I_H  512

static int g_mode = VIDEO_MODE_240P;
static int g_full_grid = 0;   // 1 = NTSC 240p uses 240 lines (full grid view)

// Effective NTSC progressive height: 240 when full-grid is active, else 224.
static int ntsc_prog_height(void)
{
    return g_full_grid ? FB_240P_FULL_H : FB_240P_H;
}

static void mode_dims(int mode, int *w, int *h, int *magh)
{
    switch (mode) {
    case VIDEO_MODE_240P: *w = FB_240P_W; *h = ntsc_prog_height(); *magh = VIDEO_MAGH_240P; break;
    case VIDEO_MODE_480I: *w = FB_480I_W; *h = FB_480I_H; *magh = VIDEO_MAGH_480I; break;
    case VIDEO_MODE_288P: *w = FB_288P_W; *h = FB_288P_H; *magh = VIDEO_MAGH_288P; break;
    case VIDEO_MODE_576I: *w = FB_576I_W; *h = FB_576I_H; *magh = VIDEO_MAGH_576I; break;
    default:              *w = FB_240P_W; *h = FB_240P_H; *magh = VIDEO_MAGH_240P; break;
    }
}

static void apply_mode_to_gs(GSGLOBAL *gs, int mode)
{
    int fb_w, fb_h, magh;
    mode_dims(mode, &fb_w, &fb_h, &magh);

    gs->Width  = fb_w;
    gs->Height = fb_h;
    if (VIDEO_IS_PROG(mode)) {
        gs->Interlace = GS_NONINTERLACED;  gs->Field = GS_FRAME;
    } else {
        gs->Interlace = GS_INTERLACED;     gs->Field = GS_FIELD;
    }
    gs->Mode  = VIDEO_IS_PAL(mode) ? GS_MODE_PAL : GS_MODE_NTSC;
    gs->PSM   = GS_PSM_CT24;
    gs->PSMZ  = GS_PSMZ_16S;
    gs->ZBuffering      = GS_SETTING_OFF;
    gs->DoubleBuffering = GS_SETTING_ON;
    gs->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_init_screen(gs);

    if (VIDEO_IS_PROG(mode)) {
        // Progressive (240p/288p): override to the conventional active width
        // (narrower than gsKit's wide default) and recenter. No scan
        // extension - solid primitives fill the framebuffer and reach the
        // edges on their own.
        int center  = gs->StartX + gs->DW / 2;
        int centerY = gs->StartY + gs->DH / 2;
        gs->MagH    = magh;
        int real_dw = fb_w * (gs->MagH + 1);
        int real_dh = fb_h * (gs->MagV + 1);
        gs->DW     = real_dw;
        gs->StartX = center - real_dw / 2;
        gs->DH     = real_dh;
        gs->StartY = centerY - real_dh / 2;
    }
    // Interlaced (480i/576i): gsKit default geometry is correct; leave as is.

    gsKit_set_display_offset(gs, 0, 0);
    gsKit_mode_switch(gs, GS_ONESHOT);

    g_mode = mode;
}

GSGLOBAL *video_init(int initial_mode)
{
    GSGLOBAL *gs = gsKit_init_global();

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    apply_mode_to_gs(gs, initial_mode);
    return gs;
}

void video_set_mode(GSGLOBAL *gs, int mode)
{
    if (mode == g_mode) return;
    apply_mode_to_gs(gs, mode);
}

// Progressive <-> interlaced within the current region.
void video_toggle_mode(GSGLOBAL *gs)
{
    g_full_grid = 0;   // never carry the full-grid height across a mode change
    int m = g_mode;
    switch (m) {
    case VIDEO_MODE_240P: m = VIDEO_MODE_480I; break;
    case VIDEO_MODE_480I: m = VIDEO_MODE_240P; break;
    case VIDEO_MODE_288P: m = VIDEO_MODE_576I; break;
    case VIDEO_MODE_576I: m = VIDEO_MODE_288P; break;
    }
    video_set_mode(gs, m);
}

// NTSC <-> PAL, keeping the progressive/interlaced choice.
void video_toggle_region(GSGLOBAL *gs)
{
    g_full_grid = 0;   // full-grid is NTSC-progressive only
    int m = g_mode;
    switch (m) {
    case VIDEO_MODE_240P: m = VIDEO_MODE_288P; break;
    case VIDEO_MODE_288P: m = VIDEO_MODE_240P; break;
    case VIDEO_MODE_480I: m = VIDEO_MODE_576I; break;
    case VIDEO_MODE_576I: m = VIDEO_MODE_480I; break;
    }
    video_set_mode(gs, m);
}

int video_current_mode(void) { return g_mode; }

int video_width(void)  { int w,h,m; mode_dims(g_mode,&w,&h,&m); return w; }
int video_height(void) { int w,h,m; mode_dims(g_mode,&w,&h,&m); return h; }

// Logical design height for pattern vertical layout in the 320-wide design
// space: 224 (NTSC) or 256 (PAL), or 240 when the NTSC full-grid view is
// active. The integer scale s = width/320 scales this to the framebuffer.
int video_design_height(void)
{
    if (VIDEO_IS_PAL(g_mode)) return FB_288P_H;          // 256
    return ntsc_prog_height();                            // 224 or 240 (full grid)
}

int video_is_pal(void) { return VIDEO_IS_PAL(g_mode); }

int video_ntsc_full_grid(void) { return g_full_grid; }

void video_set_ntsc_full_grid(GSGLOBAL *gs, int enable)
{
    // Only meaningful in NTSC progressive (240p). Ignored otherwise.
    if (!VIDEO_IS_PROG(g_mode) || VIDEO_IS_PAL(g_mode)) {
        g_full_grid = 0;
        return;
    }
    enable = enable ? 1 : 0;
    if (enable == g_full_grid) return;
    g_full_grid = enable;
    apply_mode_to_gs(gs, g_mode);   // re-init screen with the new height
}

void video_clear_black(GSGLOBAL *gs)
{
    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0, 0));
}

void video_flip(GSGLOBAL *gs)
{
    gsKit_queue_exec(gs);
    gsKit_sync_flip(gs);
}
