// Video module: GS setup, 240p/480i mode toggle with MAGH override.
#ifndef VIDEO_H
#define VIDEO_H

#include <gsKit.h>

#define VIDEO_MODE_240P  0
#define VIDEO_MODE_480I  1

// Tuned MAGH values for this project's target PVM.
// Change these if running on a different display setup.
#define VIDEO_MAGH_240P  7
#define VIDEO_MAGH_480I  3

GSGLOBAL *video_init(int initial_mode);
void      video_set_mode(GSGLOBAL *gs, int mode);
void      video_toggle_mode(GSGLOBAL *gs);
int       video_current_mode(void);
int       video_width(void);
int       video_height(void);
void      video_clear_black(GSGLOBAL *gs);
void      video_flip(GSGLOBAL *gs);

#endif