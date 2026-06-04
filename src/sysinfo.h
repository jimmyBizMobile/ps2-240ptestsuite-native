// Reads PS2 system configuration (video output type, language) so the
// menu status bar reflects the console's actual settings - like the Wii
// suite showing "Japanese Wii" / "YPbPr".
#ifndef SYSINFO_H
#define SYSINFO_H

// "RGB" or "YPbPr", from the PS2's Component Video Out setting.
const char *sysinfo_output_label(void);

// "Japanese PlayStation 2" or "PlayStation 2", from the console language.
const char *sysinfo_console_label(void);

// 1 if this is a PAL console (region 'E' = Europe), 0 otherwise (NTSC).
int sysinfo_is_pal(void);

#endif
