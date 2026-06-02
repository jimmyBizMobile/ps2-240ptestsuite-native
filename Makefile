EE_BIN  = grid.elf
EE_OBJS = src/main.o src/video.o src/pad.o src/patterns.o src/menu.o src/font.o src/sysinfo.o src/assets_data.o

EE_INCS    = -I$(GSKIT)/include -Isrc
EE_LDFLAGS = -L$(GSKIT)/lib

# No gsToolkit/fontm - text is a bitmap atlas drawn as textured quads.
EE_LIBS = -lgskit -ldmakit -lpad

EE_CFLAGS = -Wall -O2 -G0

# Always regenerate assets first so changes to PNGs / generator scripts
# are never missed due to stale timestamps on the generated files.
all: gen_assets $(EE_BIN)

.PHONY: gen_assets clean

# Force-run all generators every build. genfont rasterizes the font
# atlas; png2c embeds all PNGs (including the atlas) into assets_data.c;
# png2rects decomposes the monoscope PNG into solid rectangles
# (src/monoscope_rects.h) for procedural drawing.
gen_assets:
	python3 tools/genfont.py
	python3 tools/png2c.py
	python3 tools/png2rects.py assets/monoscope.png monoscope

# Generated sources must exist before the objects that include them build.
src/assets_data.o: gen_assets
src/patterns.o: gen_assets

clean:
	rm -f $(EE_OBJS) $(EE_BIN) src/assets_data.c src/assets_data.h \
	      src/font_data.h src/monoscope_rects.h assets/font_atlas.png

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal