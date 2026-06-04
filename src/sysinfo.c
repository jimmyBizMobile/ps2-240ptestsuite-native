#include "sysinfo.h"
#include <osd_config.h>
#include <kernel.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// GetOsdConfigParam is a kernel syscall that fills a ConfigParam struct
// (defined in osd_config.h) describing the user's system settings.
// videoOutput: 0 = RGB (scart), 1 = component (YPbPr)
// language / japLanguage: used to tell if this is a Japanese console.

const char *sysinfo_output_label(void)
{
    ConfigParam cfg;
    GetOsdConfigParam(&cfg);
    return (cfg.videoOutput == VIDEO_OUTPUT_COMPONENT) ? "YPbPr" : "RGB";
}

// ROMVER lives in the boot ROM as "VVVVRTYYYYMMDD"; char index 4 is the
// hardware region, independent of the OSD language the user picked.
// Read it once and cache it.
static char sysinfo_region_char(void)
{
    static int cached = 0;
    static char region = '?';
    if (cached) return region;
    cached = 1;

    char romver[16];
    int fd = open("rom0:ROMVER", O_RDONLY);
    if (fd >= 0) {
        read(fd, romver, 14);
        romver[14] = '\0';
        close(fd);
        region = romver[4];
    }
    return region;
}

// PAL consoles are the European region ('E'). All others (J/A/H/C/R) are
// NTSC. (If the region can't be read, default to NTSC.)
int sysinfo_is_pal(void)
{
    return sysinfo_region_char() == 'E';
}

const char *sysinfo_console_label(void)
{
    static char label[32];
    static int initialized = 0;
    if (initialized) return label;
    initialized = 1;

    char region = sysinfo_region_char();
    if (region != '?') {
        const char *r;
        switch (region) {
            case 'J': r = "Japan";        break;
            case 'A': r = "USA";          break;
            case 'E': r = "Europe";       break;
            case 'H': r = "Asia";         break;
            case 'C': r = "China";        break;
            case 'R': r = "Russia";       break;
            default:  r = "Unknown";      break;
        }
        snprintf(label, sizeof label, "PlayStation 2 (%s)", r);
    } else {
        snprintf(label, sizeof label, "PlayStation 2");
    }
    return label;
}
