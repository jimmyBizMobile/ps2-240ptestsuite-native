// Video module: NTSC/PAL, progressive/interlaced mode handling.
#ifndef VIDEO_H
#define VIDEO_H

#include <gsKit.h>

// Modes: NTSC 240p/480i and PAL 288p/576i. Low modes are progressive,
// high modes interlaced. (The labels 240p/480i/288p/576i are the standard
// mode names; the actual framebuffers use safe-area heights - see video.c.)
#define VIDEO_MODE_240P  0   // NTSC progressive
#define VIDEO_MODE_480I  1   // NTSC interlaced
#define VIDEO_MODE_288P  2   // PAL progressive
#define VIDEO_MODE_576I  3   // PAL interlaced

#define VIDEO_IS_PAL(m)   ((m) == VIDEO_MODE_288P || (m) == VIDEO_MODE_576I)
#define VIDEO_IS_PROG(m)  ((m) == VIDEO_MODE_240P || (m) == VIDEO_MODE_288P)

// Tuned MAGH for this project's target PVM. Horizontal active width is
// similar between NTSC and PAL, so PAL uses the same MAGH as NTSC.
#define VIDEO_MAGH_240P  7
#define VIDEO_MAGH_288P  7
#define VIDEO_MAGH_480I  3
#define VIDEO_MAGH_576I  3

GSGLOBAL *video_init(int initial_mode);
void      video_set_mode(GSGLOBAL *gs, int mode);
void      video_toggle_mode(GSGLOBAL *gs);    // progressive <-> interlaced (same region)
void      video_toggle_region(GSGLOBAL *gs);  // NTSC <-> PAL (keeps prog/interlaced)
int       video_current_mode(void);
int       video_width(void);
int       video_height(void);          // real framebuffer height of current mode
int       video_design_height(void);   // logical design height for patterns (224 NTSC / 256 PAL)
int       video_is_pal(void);
// Full-grid override: when enabled in NTSC 240p, the progressive framebuffer
// uses 240 lines (15 clean 16px rows) instead of 224. Applies only to NTSC
// progressive; ignored in 480i and PAL. Re-applies the current mode.
void      video_set_ntsc_full_grid(GSGLOBAL *gs, int enable);
int       video_ntsc_full_grid(void);
void      video_clear_black(GSGLOBAL *gs);
void      video_flip(GSGLOBAL *gs);

#endif
