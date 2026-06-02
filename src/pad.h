// Controller input with edge-detected button press.
#ifndef PAD_H
#define PAD_H

#include <libpad.h>

int  pad_init(void);
void pad_update(void);
int  pad_pressed(int btn);   // 1 on rising edge, 0 otherwise

#endif