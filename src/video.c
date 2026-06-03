#include "video.h"
#include <gsKit.h>
#include <dmaKit.h>

#define FB_240P_W  320
#define FB_240P_H  224
#define FB_480I_W  640
#define FB_480I_H  448

static int g_mode = VIDEO_MODE_240P;

static void apply_mode_to_gs(GSGLOBAL *gs, int mode)
{
    if (mode == VIDEO_MODE_240P) {
        gs->Width = FB_240P_W;  gs->Height = FB_240P_H;
        gs->Interlace = GS_NONINTERLACED;  gs->Field = GS_FRAME;
    } else {
        gs->Width = FB_480I_W;  gs->Height = FB_480I_H;
        gs->Interlace = GS_INTERLACED;  gs->Field = GS_FIELD;
    }
    gs->Mode  = GS_MODE_NTSC;
    gs->PSM   = GS_PSM_CT24;
    gs->PSMZ  = GS_PSMZ_16S;
    gs->ZBuffering      = GS_SETTING_OFF;
    gs->DoubleBuffering = GS_SETTING_ON;
    gs->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_init_screen(gs);

    if (mode == VIDEO_MODE_240P) {
        int center  = gs->StartX + gs->DW / 2;
        int centerY = gs->StartY + gs->DH / 2;
        gs->MagH    = VIDEO_MAGH_240P;
        int real_dw = FB_240P_W * (gs->MagH + 1);
        int real_dh = FB_240P_H * (gs->MagV + 1);
        //before  gs->DW = real_dw + (gs->MagH + 1);   // +1 column anti-clip
        gs->DW = real_dw;
        gs->StartX = center - real_dw / 2;
        //before gs->DH = real_dh + (gs->MagV + 1);   // +1 row anti-clip
        gs->DH = real_dh;
        gs->StartY = centerY - real_dh / 2;
    }

    // In 240p the last framebuffer column/row lands exactly on the display
    // scan boundary (StartX+DW / StartY+DH) and gets clipped. 480i has
    // margin and is fine. For 240p ONLY, extend the scan by one
    // magnification unit on each axis so the final column/row is displayed.
    // (Patterns are now drawn with solid primitives, which fill the
    // framebuffer fully; this ensures that framebuffer reaches the screen.)

    /*int center  = gs->StartX + gs->DW / 2;
    int centerY = gs->StartY + gs->DH / 2;
    int magh    = (mode == VIDEO_MODE_240P) ? VIDEO_MAGH_240P : VIDEO_MAGH_480I;
    int fb_w    = (mode == VIDEO_MODE_240P) ? FB_240P_W : FB_480I_W;
    int magv    = gs->MagV;
    gs->MagH = magh;

    if (mode == VIDEO_MODE_240P) {
        int real_dw = fb_w * (magh + 1);
        int real_dh = FB_240P_H * (magv + 1);
        gs->DW     = real_dw + (magh + 1);   // extend right scan by one column
        gs->StartX = center - real_dw / 2;
        gs->DH     = real_dh + (magv + 1);   // extend bottom scan by one row
        gs->StartY = centerY - real_dh / 2;
    } 
    else {
        // 480i: original working behavior, untouched.
        gs->DW     = fb_w * (magh + 1);
        gs->StartX = center - gs->DW / 2;
    }
        */
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

void video_toggle_mode(GSGLOBAL *gs)
{
    video_set_mode(gs, (g_mode == VIDEO_MODE_240P) ? VIDEO_MODE_480I : VIDEO_MODE_240P);
}

int video_current_mode(void) { return g_mode; }
int video_width(void)  { return (g_mode == VIDEO_MODE_240P) ? FB_240P_W : FB_480I_W; }
int video_height(void) { return (g_mode == VIDEO_MODE_240P) ? FB_240P_H : FB_480I_H; }

void video_clear_black(GSGLOBAL *gs)
{
    gsKit_clear(gs, GS_SETREG_RGBAQ(0, 0, 0, 0, 0));
}

void video_flip(GSGLOBAL *gs)
{
    gsKit_queue_exec(gs);
    gsKit_sync_flip(gs);
}