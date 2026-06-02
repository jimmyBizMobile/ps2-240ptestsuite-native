#include "font.h"
#include "font_data.h"
#include "assets_data.h"
#include <gsKit.h>
#include <stdint.h>
#include <string.h>
#include <kernel.h>

static GSTEXTURE g_atlas;

void font_init(GSGLOBAL *gs)
{
    memset(&g_atlas, 0, sizeof(g_atlas));
    g_atlas.Width  = ASSET_FONT_ATLAS_W;
    g_atlas.Height = ASSET_FONT_ATLAS_H;
    g_atlas.PSM    = GS_PSM_CT32;
    g_atlas.Mem    = (u32 *)asset_font_atlas_pixels;
    g_atlas.Filter = GS_FILTER_NEAREST;
    g_atlas.Delayed = 0;
    g_atlas.TBW = (ASSET_FONT_ATLAS_W + 63) / 64;

    FlushCache(0);
    g_atlas.Vram = gsKit_vram_alloc(gs,
        gsKit_texture_size(g_atlas.Width, g_atlas.Height, g_atlas.PSM),
        GSKIT_ALLOC_USERBUFFER);
    if (g_atlas.Vram != GSKIT_ALLOC_ERROR)
        gsKit_texture_upload(gs, &g_atlas);
}

static const FontGlyph *glyph_for(char c)
{
    if (c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR) return NULL;
    return &font_glyphs[(int)c - FONT_FIRST_CHAR];
}

float font_text_width(const char *text, float scale)
{
    float w = 0.0f;
    for (const char *p = text; *p; p++) {
        const FontGlyph *g = glyph_for(*p);
        if (g) w += g->advance * scale;
    }
    return w;
}

void font_print(GSGLOBAL *gs, float x, float y, float scale, u64 color,
                const char *text)
{
    // Enable alpha so transparent atlas areas don't paint over the
    // background. The atlas is generated with RGB forced to 0 wherever
    // alpha is 0, so transparent texels contribute nothing.
    gsKit_set_primalpha(gs, GS_SETREG_ALPHA(0, 1, 0, 1, 0), 0);
    gs->PrimAlphaEnable = GS_SETTING_ON;

    float pen_x = x;
    for (const char *p = text; *p; p++) {
        const FontGlyph *g = glyph_for(*p);
        if (!g) continue;

        // The space character (and any glyph with no visible pixels) must
        // not draw a quad - its atlas rectangle may sample a stray texel
        // and show as a dot. Just advance the pen for whitespace.
        if (*p == ' ') {
            pen_x += g->advance * scale;
            continue;
        }

        float w = g->w * scale;
        float h = g->h * scale;

        gsKit_prim_sprite_texture(gs, &g_atlas,
            pen_x,       y,                       // screen top-left
            (float)g->x, (float)g->y,             // atlas top-left (UV)
            pen_x + w,   y + h,                    // screen bottom-right
            (float)(g->x + g->w), (float)(g->y + g->h),  // atlas bottom-right
            2, color);

        pen_x += g->advance * scale;
    }

    gs->PrimAlphaEnable = GS_SETTING_OFF;
}