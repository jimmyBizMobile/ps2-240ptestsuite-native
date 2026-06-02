#include "pad.h"
#include <libpad.h>
#include <kernel.h>

static char g_pad_buf[256] __attribute__((aligned(64)));
static unsigned int g_prev = 0, g_curr = 0;

int pad_init(void)
{
    padInit(0);
    if (padPortOpen(0, 0, g_pad_buf) == 0) return -1;
    int s = padGetState(0, 0);
    while (s != PAD_STATE_DISCONN && s != PAD_STATE_STABLE && s != PAD_STATE_FINDCTP1)
        s = padGetState(0, 0);
    return (s == PAD_STATE_DISCONN) ? -1 : 0;
}

void pad_update(void)
{
    struct padButtonStatus btns;
    g_prev = g_curr;
    int s = padGetState(0, 0);
    if ((s == PAD_STATE_STABLE || s == PAD_STATE_FINDCTP1) && padRead(0, 0, &btns) != 0) {
        g_curr = 0xFFFF ^ btns.btns;
    } else {
        g_curr = 0;
    }
}

int pad_pressed(int btn) { return (g_curr & btn) && !(g_prev & btn); }