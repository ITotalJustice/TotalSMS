#include "sms.h"
#include "sms_internal.h"
#include "sms_types.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>


#define VDP sms->vdp

#ifndef SMS_PIXEL_WIDTH
    typedef uint32_t pixel_width_t;
#else
    #if SMS_PIXEL_WIDTH == 32
        typedef uint32_t pixel_width_t;
    #elif SMS_PIXEL_WIDTH == 16
        typedef uint16_t pixel_width_t;
    #elif SMS_PIXEL_WIDTH == 8
        typedef uint8_t pixel_width_t;
    #else
        #error "invalid SMS_PIXEL_WIDTH! use [8,16,32]"
    #endif
#endif

enum
{
    NTSC_HCOUNT_MAX = 342,
    NTSC_VCOUNT_MAX = 262,

    NTSC_FPS = 60,

    NTSC_CYCLES_PER_LINE = 228, // SMS_CPU_CLOCK / NTSC_VCOUNT_MAX / NTSC_FPS, // ~228

    SPRITE_EOF = 208,
};

// SOURCE: https://www.smspower.org/forums/8161-SMSDisplayTiming
// (divide mclks by 3)
// 59,736 (179,208 mclks, 228x262)
// frame_int 202 cycles into line 192 (607 mclks)
// line_int 202 cycles into triggering (608 mclks)

static FORCE_INLINE bool vdp_is_line_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 4);
}

static FORCE_INLINE bool vdp_is_vblank_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 5);
}

static FORCE_INLINE bool vdp_is_screen_size_change_enabled(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 1);
}

static FORCE_INLINE uint16_t vdp_get_nametable_base_addr(const struct SMS_Core* sms)
{
    if (SMS_is_system_type_sg(sms))
    {
        return (VDP.registers[0x2] & 0xF) << 10;
    }
    else
    {
        return (VDP.registers[0x2] & 0xE) << 10;
    }
}

static FORCE_INLINE uint16_t vdp_get_sprite_attribute_base_addr(const struct SMS_Core* sms)
{
    if (SMS_is_system_type_sg(sms))
    {
        return (VDP.registers[0x5] & 0x7F) * 128;
    }
    else
    {
        return (VDP.registers[0x5] & 0x7E) * 128;
    }
}

static FORCE_INLINE bool vdp_get_sprite_pattern_select(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x6], 2);
}

static FORCE_INLINE bool vdp_is_display_enabled(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 6);
}

static FORCE_INLINE uint8_t vdp_get_sprite_height(const struct SMS_Core* sms)
{
    const bool doubled_sprites = IS_BIT_SET(VDP.registers[0x1], 0);
    const uint8_t sprite_size = IS_BIT_SET(VDP.registers[0x1], 1) ? 16 : 8;

    return sprite_size << doubled_sprites;
}

// returns the hieght of the screen
static uint16_t vdp_get_screen_height(const struct SMS_Core* sms)
{
    if (!vdp_is_screen_size_change_enabled(sms))
    {
        return 192;
    }
    else if (IS_BIT_SET(VDP.registers[1], 4))
    {
        return 224;
    }
    else if (IS_BIT_SET(VDP.registers[1], 3))
    {
        return 240;
    }
    else
    {
        return 192;
    }
}

static uint8_t vdp_get_overscan_colour(const struct SMS_Core* sms)
{
    return VDP.registers[0x7] & 0xF;
}

struct VDP_region
{
    uint16_t startx;
    uint16_t endx;
    uint16_t pixelx;
    uint16_t pixely;
};

static FORCE_INLINE struct VDP_region vdp_get_region(const struct SMS_Core* sms)
{
    if (SMS_is_system_type_gg(sms))
    {
        return (struct VDP_region)
        {
            .startx = 48,
            .endx = 160 + 48,
            .pixelx = 62 - 48,
            .pixely = VDP.vcount + 27,
        };
    }
    else
    {
        return (struct VDP_region)
        {
            .startx = IS_BIT_SET(VDP.registers[0x0], 5) ? 8 : 0,
            .endx = SMS_SCREEN_WIDTH,
            .pixelx = 0,
            .pixely = VDP.vcount,
        };
    }
}

static FORCE_INLINE bool vdp_is_display_active(const struct SMS_Core* sms)
{
    if (SMS_is_system_type_gg(sms))
    {
        return VDP.vcount >= 24 && VDP.vcount < 144 + 24;
    }
    else
    {
        return VDP.vcount < 192;
    }
}

#ifndef SMS_PIXEL_WIDTH
static void write_scanline_to_frame(struct SMS_Core* sms, const pixel_width_t* scanline, const uint8_t y)
{
    switch (sms->bpp)
    {
        case 1:
        case 8: {
            uint8_t* pixels = &((uint8_t*)sms->pixels)[sms->pitch * y];
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint8_t)scanline[i];
            }
        }   break;

        case 2:
        case 15:
        case 16: {
            uint16_t* pixels = ((uint16_t*)sms->pixels) + (sms->pitch * y);
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint16_t)scanline[i];
            }
        }   break;

        case 4:
        case 24:
        case 32: {
            uint32_t* pixels = &((uint32_t*)sms->pixels)[sms->pitch * y];
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint32_t)scanline[i];
            }
        }   break;

        default:
            assert(!"bpp invalid option!");
            break;
    }
}
#endif

uint8_t vdp_status_flag_read(struct SMS_Core* sms)
{
    if (SMS_is_system_type_sg(sms))
    {
        uint8_t v = 0;

        v |= VDP.frame_interrupt_pending << 7;
        v |= VDP.sprite_overflow << 6;
        v |= VDP.sprite_collision << 5;
        v |= VDP.fifth_sprite_num;

        // these are reset on read
        VDP.frame_interrupt_pending = false;
        VDP.sprite_overflow = false;
        VDP.sprite_collision = false;
        VDP.fifth_sprite_num = 0;

        return v;
    }
    else
    {
        uint8_t v = 31; // unused bits 1,2,3,4

        v |= VDP.frame_interrupt_pending << 7;
        v |= VDP.sprite_overflow << 6;
        v |= VDP.sprite_collision << 5;

        // these are reset on read
        VDP.frame_interrupt_pending = false;
        VDP.line_interrupt_pending = false;
        VDP.sprite_overflow = false;
        VDP.sprite_collision = false;

        return v;
    }
}

void vdp_io_write(struct SMS_Core* sms, const uint8_t addr, const uint8_t value)
{
    if (SMS_is_system_type_sg(sms))
    {
        VDP.registers[addr & 0x7] = value;
    }
    else
    {
        switch (addr & 0xF)
        {
            case 0x0:
                assert(IS_BIT_SET(value, 2) && "not mode4, using TMS9918 modes!");
                assert(IS_BIT_SET(value, 1) && !IS_BIT_SET(VDP.registers[1], 3) && "240 height mode set");
                assert(IS_BIT_SET(value, 1) && !IS_BIT_SET(VDP.registers[1], 4) && "224 height mode set");
                break;

            case 0x1:
                assert(!IS_BIT_SET(value, 3) && IS_BIT_SET(VDP.registers[0], 1) && "240 height mode set");
                assert(!IS_BIT_SET(value, 4) && IS_BIT_SET(VDP.registers[0], 1) && "224 height mode set");
                break;

            case 0x3:
                assert(value == 0xFF && "colour table bits not all set");
                break;

            // unused registers
            case 0xB:
            case 0xC:
            case 0xD:
            case 0xE:
            case 0xF:
                return;
        }

        VDP.registers[addr & 0xF] = value;
    }
}

// same as i used in dmg / gbc rendering for gb
struct PriorityBuf
{
    bool array[SMS_SCREEN_WIDTH];
};

static void vdp_mode2_render_background(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;
    const uint8_t overscan_colour = vdp_get_overscan_colour(sms);

    const uint16_t pattern_table_addr = (VDP.registers[4] & 0x04) << 11;
    const uint16_t colour_map_addr = (VDP.registers[3] & 0x80) << 6;
    const uint16_t region = (VDP.registers[4] & 0x03) << 8;

    for (uint8_t col = 0; col < 32; col++)
    {
        const uint16_t tile_number = (row * 32) + col;
        const uint16_t name_tile_addr = vdp_get_nametable_base_addr(sms) + tile_number;
        const uint16_t name_tile = VDP.vram[name_tile_addr] | (region & 0x300 & tile_number);

        const uint8_t pattern_line = VDP.vram[pattern_table_addr + (name_tile * 8) + fine_line];
        const uint8_t color_line = VDP.vram[colour_map_addr + (name_tile * 8) + fine_line];

        const uint8_t bg_color = color_line & 0x0F;
        const uint8_t fg_color = color_line >> 4;

        for (uint8_t x = 0; x < 8; x++)
        {
            const uint8_t x_index = (col * 8) + x;

            uint8_t colour = IS_BIT_SET(pattern_line, 7 - x) ? fg_color : bg_color;
            colour = (colour > 0) ? colour : overscan_colour;
            scanline[x_index] = sms->vdp.colour[colour];
        }
    }
}

static void vdp_mode1_render_background(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;
    const uint8_t overscan_colour = vdp_get_overscan_colour(sms);

    const uint16_t name_table_addr = (VDP.registers[2] & 0x0F) << 10;
    const uint16_t pattern_table_addr = (VDP.registers[4] & 0x07) << 11;
    const uint16_t colour_map_addr = VDP.registers[3] << 6;

    for (uint8_t col = 0; col < 32; col++)
    {
        const uint16_t tile_number = (row * 32) + col;
        const uint16_t name_tile_addr = name_table_addr + tile_number;
        const uint16_t name_tile = VDP.vram[name_tile_addr];

        const uint8_t pattern_line = VDP.vram[pattern_table_addr + (name_tile * 8) + fine_line];
        const uint8_t color_line = VDP.vram[colour_map_addr + (name_tile >> 3)];

        const uint8_t bg_color = color_line & 0x0F;
        const uint8_t fg_color = color_line >> 4;

        for (uint8_t x = 0; x < 8; x++)
        {
            const uint8_t x_index = (col * 8) + x;

            uint8_t colour = IS_BIT_SET(pattern_line, 7 - x) ? fg_color : bg_color;
            colour = (colour > 0) ? colour : overscan_colour;
            scanline[x_index] = sms->vdp.colour[colour];
        }
    }
}

static inline struct CachedPalette vdp_get_palette(struct SMS_Core* sms, const uint16_t pattern_index)
{
    struct CachedPalette* cpal = &VDP.cached_palette[pattern_index >> 2];

    // check if one of the 4 bit_planes have changed
    if (VDP.dirty_vram[pattern_index >> 2])
    {
        VDP.dirty_vram[pattern_index >> 2] = false;

        const uint8_t bit_plane0 = VDP.vram[pattern_index + 0];
        const uint8_t bit_plane1 = VDP.vram[pattern_index + 1];
        const uint8_t bit_plane2 = VDP.vram[pattern_index + 2];
        const uint8_t bit_plane3 = VDP.vram[pattern_index + 3];

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t bit_flip = x;
            const uint8_t bit_norm = 7 - x;

            cpal->flipped <<= 4;
            cpal->flipped |= IS_BIT_SET(bit_plane0, bit_flip) << 0;
            cpal->flipped |= IS_BIT_SET(bit_plane1, bit_flip) << 1;
            cpal->flipped |= IS_BIT_SET(bit_plane2, bit_flip) << 2;
            cpal->flipped |= IS_BIT_SET(bit_plane3, bit_flip) << 3;

            cpal->normal <<= 4;
            cpal->normal |= IS_BIT_SET(bit_plane0, bit_norm) << 0;
            cpal->normal |= IS_BIT_SET(bit_plane1, bit_norm) << 1;
            cpal->normal |= IS_BIT_SET(bit_plane2, bit_norm) << 2;
            cpal->normal |= IS_BIT_SET(bit_plane3, bit_norm) << 3;
        }
    }

    return *cpal;
}

static void vdp_render_background(struct SMS_Core* sms, pixel_width_t* scanline, struct PriorityBuf* prio)
{
    const struct VDP_region region = vdp_get_region(sms);

    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;

    // check if horizontal scrolling should be disabled
    // doesn't work in GG mode as the screen centered
    const bool horizontal_disabled = IS_BIT_SET(VDP.registers[0x0], 6) && line < 16;

    const uint8_t starting_col = (32 - (VDP.registers[0x8] >> 3)) & 31;
    const uint8_t fine_scrollx = horizontal_disabled ? 0 : VDP.registers[0x8] & 0x7;
    const uint8_t horizontal_scroll = horizontal_disabled ? 0 : starting_col;

    const uint8_t starting_row = VDP.vertical_scroll >> 3;
    const uint8_t fine_scrolly = VDP.vertical_scroll & 0x7;

    const uint8_t* nametable = NULL;
    uint8_t check_col = 0;
    uint8_t palette_index_offset = 0;

    // if set, we use the internal col counter, else, the starting_col
    if (!IS_BIT_SET(VDP.registers[0x1], 7))
    {
        check_col = (starting_col) & 31;
    }

    {
        // we need to check if we cross the next row
        const bool next_row = (fine_line + fine_scrolly) > 7;

        const uint16_t vertical_offset = ((row + starting_row + next_row) % 28) * 64;
        palette_index_offset = (fine_line + fine_scrolly) & 0x7;
        nametable = &VDP.vram[vdp_get_nametable_base_addr(sms) + vertical_offset];
    }

    if (region.startx == 8)
    {
        // render overscan
        const uint8_t palette_index = 16 + vdp_get_overscan_colour(sms);

        for (int x_index = 0; x_index < 8; x_index++)
        {
            // used when sprite rendering, will skip if prio set and not pal0
            prio->array[x_index] = true;//priority && palette_index != 0;
            scanline[x_index] = VDP.colour[palette_index];
        }
    }

    for (uint8_t col = 0; col < 32; ++col)
    {
        check_col = (check_col + 1) & 31;
        const uint16_t horizontal_offset = ((horizontal_scroll + col) & 31) * 2;

        // check if vertical scrolling should be disabled
        if (IS_BIT_SET(VDP.registers[0x0], 7) && check_col >= 24)
        {
            const uint16_t vertical_offset = (row % 28) * 64;
            palette_index_offset = fine_line;
            nametable = &VDP.vram[vdp_get_nametable_base_addr(sms) + vertical_offset];
        }

        const uint16_t tile =
            nametable[horizontal_offset + 0] << 0 |
            nametable[horizontal_offset + 1] << 8 ;

        // if set, background will display over sprites
        const bool priority = IS_BIT_SET(tile, 12);
        // select from either sprite or background palette
        const bool palette_select = IS_BIT_SET(tile, 11);
        // vertical flip
        const bool vertical_flip = IS_BIT_SET(tile, 10);
        // horizontal flip
        const bool horizontal_flip = IS_BIT_SET(tile, 9);
        // one of the 512 patterns to select
        uint16_t pattern_index = (tile & 0x1FF) * 32;

        if (vertical_flip)
        {
            pattern_index += (7 - palette_index_offset) * 4;
        }
        else
        {
            pattern_index += palette_index_offset * 4;
        }

        const struct CachedPalette cpal = vdp_get_palette(sms, pattern_index);
        const uint32_t palette = horizontal_flip ? cpal.flipped : cpal.normal;
        const uint8_t pal_base = palette_select ? 16 : 0;

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t x_index = ((col * 8) + x + fine_scrollx) % SMS_SCREEN_WIDTH;

            if (x_index >= region.endx)
            {
                break;
            }

            if (x_index < region.startx)
            {
                continue;
            }

            const uint8_t palette_index =  (palette >> (28 - (4 * x))) & 0xF;
            prio->array[x_index] = priority && palette_index != 0;
            scanline[x_index] = VDP.colour[pal_base + palette_index];
        }
    }
}

void SMS_get_pixel_region(const struct SMS_Core* sms, int* x, int* y, int* w, int* h)
{
    // todo: support different height modes
    if (SMS_is_system_type_gg(sms))
    {
        *x = 48;
        *y = 24;
        *w = 160;
        *h = 144;
    }
    else
    {
        *x = 0;
        *y = 0;
        *w = SMS_SCREEN_WIDTH;
        *h = SMS_SCREEN_HEIGHT;
    }
}

struct SgSpriteEntry
{
    int16_t y;
    int16_t x;
    uint8_t tile_num;
    uint8_t colour;
};

struct SgSpriteEntries
{
    struct SgSpriteEntry entry[32];
    uint8_t count;
};

static struct SgSpriteEntries vdp_parse_sg_sprites(struct SMS_Core* sms)
{
    if (!SMS_is_system_type_sg(sms))
    {
        assert(IS_BIT_SET(VDP.registers[0x5], 0) && "needs lower index for oam");
        assert((VDP.registers[0x6] & 0x3) == 0x3 && "Sprite Pattern Generator Base Address");
    }

    struct SgSpriteEntries sprites = {0};

    const uint8_t line = VDP.vcount;
    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
    const uint8_t sprite_size = vdp_get_sprite_height(sms);

    for (uint8_t i = 0; i < 128; i += 4)
    {
        // todo: find out how the sprite y wrapping works!
        int16_t y = VDP.vram[sprite_attribute_base_addr + i + 0] + 1;

        // special number used to stop sprite parsing!
        if (y == SPRITE_EOF + 1)
        {
            break;
        }

        if (y > 192)
        {
            y -= 256;
        }

        if (line >= y && line < (y + sprite_size))
        {
            const int16_t x = VDP.vram[sprite_attribute_base_addr + i + 1];
            const uint8_t tile_num = VDP.vram[sprite_attribute_base_addr + i + 2];
            const uint8_t colour = VDP.vram[sprite_attribute_base_addr + i + 3];

            sprites.entry[sprites.count].y = y;
            // note: docs say its shifted 32 pixels NOT 8!
            sprites.entry[sprites.count].x = x - (IS_BIT_SET(colour, 7) ? 32 : 0);
            sprites.entry[sprites.count].tile_num = sprite_size > 8 ? tile_num & ~0x3 : tile_num;
            sprites.entry[sprites.count].colour = colour & 0xF;
            sprites.count++;
        }
    }

    return sprites;
}

static void vdp_mode1_render_sprites(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint16_t tile_addr = (VDP.registers[0x6] & 0x7) * 0x800;
    const uint8_t sprite_size = vdp_get_sprite_height(sms);
    const struct SgSpriteEntries sprites = vdp_parse_sg_sprites(sms);
    bool drawn_sprites[SMS_SCREEN_WIDTH] = {0};
    uint8_t sprite_rendered_count = 0;

    for (uint8_t i = 0; i < sprites.count; i++)
    {
        const struct SgSpriteEntry* sprite = &sprites.entry[i];

        if (sprite->colour == 0)
        {
            continue;
        }

        uint8_t pattern_line = VDP.vram[tile_addr + (line - sprite->y) + (sprite->tile_num * 8)];
        // check if we actually drawn a sprite
        bool did_draw_a_sprite = false;

        for (uint8_t x = 0; x < sprite_size; x++)
        {
            const int16_t x_index = x + sprite->x;

            // skip if column0 or offscreen
            if (x_index < 0)
            {
                continue;
            }

            if (x_index >= SMS_SCREEN_WIDTH)
            {
                break;
            }

            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // idk what name to give this
            // basically, a sprite can be 32x32 but it doubles
            // up on the bits so that bit0 will be for pixel 0,1
            const uint8_t x2 = x >> (sprite_size == 32);

            if (x2 == 8)
            {
                // reload
                pattern_line = VDP.vram[tile_addr + (line - sprite->y) + (sprite->tile_num * 8) + 16];
            }

            if (!IS_BIT_SET(pattern_line, 7 - (x2 & 7)))
            {
                continue;
            }

            if (sprite_rendered_count < 4) // max of 4 sprites lol
            {
                did_draw_a_sprite = true;
                drawn_sprites[x_index] = true;
                scanline[x_index] = sms->vdp.colour[sprite->colour];//sms->colour_callback(NULL, c.r, c.g, c.b);
            }
        }

        // update count if we drawn a sprite
        sprite_rendered_count += did_draw_a_sprite;
        // now check if theres more than 5 spirtes on a line
        if (i >= 4)
        {
            VDP.sprite_overflow = true;
            VDP.fifth_sprite_num = i;
        }
    }
}

struct SpriteEntry
{
    int16_t y;
    uint8_t xn_index;
};

struct SpriteEntries
{
    struct SpriteEntry entry[8];
    uint8_t count;
};

static struct SpriteEntries vdp_parse_sprites(struct SMS_Core* sms)
{
    // assert((VDP.registers[0x6] & 0x3) == 0x3 && "Sprite Pattern Generator Base Address");

    struct SpriteEntries sprites = {0};

    const uint8_t line = VDP.vcount;
    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
    const uint8_t sprite_attribute_x_index = IS_BIT_SET(VDP.registers[0x5], 0) ? 128 : 0;
    const uint8_t sprite_size = vdp_get_sprite_height(sms);

    for (uint8_t i = 0; i < 64; ++i)
    {
        // todo: find out how the sprite y wrapping works!
        int16_t y = VDP.vram[sprite_attribute_base_addr + i] + 1;

        // special number used to stop sprite parsing!
        if (y == SPRITE_EOF + 1)
        {
            break;
        }

        // docs (TMS9918.pdf) state that y is partially signed (-31, 0) which
        // is line 224 im not sure if this is correct, as it likely isn't as
        // theres a 240 hieght mode, meaning no sprites could be displayed past 224...
        // so maybe this should reset on 240? or maybe it depends of the
        // the height mode selected, ie, 192, 224 and 240
        #if 0
        if (y > 224)
        #else
        if (y > 192)
        #endif
        {
            y -= 256;
        }

        if (line >= y && line < (y + sprite_size))
        {
            if (sprites.count < 8)
            {
                sprites.entry[sprites.count].y = y;
                // xn values are either low (0) or high (128) end of SAT
                sprites.entry[sprites.count].xn_index = sprite_attribute_x_index + (i * 2);
                sprites.count++;
            }

            // if we have filled the sprite array, we need to keep checking further
            // entries just in case another sprite falls on the same line, in which
            // case, the sprite overflow flag is set for stat.
            else
            {
                if (!SMS_is_system_type_sg(sms))
                {
                    VDP.sprite_overflow = true;
                }
                break;
            }
        }
    }

    return sprites;
}

static void vdp_render_sprites(struct SMS_Core* sms, pixel_width_t* scanline, const struct PriorityBuf* prio)
{
    const struct VDP_region region = vdp_get_region(sms);

    const uint8_t line = VDP.vcount;
    const uint16_t attr_addr = vdp_get_sprite_attribute_base_addr(sms);
    // if set, we fetch patterns from upper table
    const uint16_t pattern_select = vdp_get_sprite_pattern_select(sms) ? 256 : 0;
    // if set, sprites start 8 to the left
    const int8_t sprite_x_offset = IS_BIT_SET(VDP.registers[0x0], 3) ? -8 : 0;

    const struct SpriteEntries sprites = vdp_parse_sprites(sms);

    bool drawn_sprites[SMS_SCREEN_WIDTH] = {0};

    for (uint8_t i = 0; i < sprites.count; ++i)
    {
        const struct SpriteEntry* sprite = &sprites.entry[i];

        // signed because the sprite can be negative if -8!
        const int16_t sprite_x = VDP.vram[attr_addr + sprite->xn_index + 0] + sprite_x_offset;

        if (sprite_x+8 < region.startx || sprite_x>=region.endx)
        {
            continue;
        }

        uint16_t pattern_index = VDP.vram[attr_addr + sprite->xn_index + 1] + pattern_select;

        // docs state that if bit1 of reg1 is set (should always), bit0 is ignored
        // i am not sure if this is applied to the final value of the value
        // initial value however...
        if (IS_BIT_SET(VDP.registers[0x1], 1))
        {
            pattern_index &= ~0x1;
        }

        pattern_index *= 32;

        // this has already taken into account of sprite size when parsing
        // sprites, so for example:
        // - line = 3,
        // - sprite->y = 1,
        // - sprite_size = 8,
        // then y will be accepted for this line, but needs to be offset from
        // the current line we are on, so line-sprite->y = 2
        pattern_index += (line - sprite->y) * 4;

        const struct CachedPalette cpal = vdp_get_palette(sms, pattern_index);
        const uint32_t palette = cpal.normal;//horizontal_flip ? cpal.flipped : cpal.normal;

        // note: the order of the below ifs are important.
        // opaque sprites can collide, even when behind background/
        for (uint8_t x = 0; x < 8; ++x)
        {
            const int16_t x_index = x + sprite_x;

            // skip if column0 or offscreen
            if (x_index < region.startx)
            {
                continue;
            }

            if (x_index >= region.endx)
            {
                break;
            }

            const uint8_t palette_index = (palette >> (28 - (4 * x))) & 0xF;

            // for sprites, pal0 is transparent
            if (palette_index == 0)
            {
                continue;
            }

            // skip if we already rendered a sprite!
            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // keep track of this sprite already being rendered
            drawn_sprites[x_index] = true;

            // skip is bg has priority
            if (prio->array[x_index])
            {
                continue;
            }

            // sprite cram index is the upper 16-bytes!
            scanline[x_index] = VDP.colour[palette_index + 16];
        }
    }
}

static void vdp_update_sms_colours(struct SMS_Core* sms)
{
    assert(sms->vdp.dirty_cram_max <= 32);

    for (int i = sms->vdp.dirty_cram_min; i < sms->vdp.dirty_cram_max; i++)
    {
        if (sms->vdp.dirty_cram[i])
        {
            const uint8_t r = (sms->vdp.cram[i] >> 0) & 0x3;
            const uint8_t g = (sms->vdp.cram[i] >> 2) & 0x3;
            const uint8_t b = (sms->vdp.cram[i] >> 4) & 0x3;

            sms->vdp.colour[i] = sms->colour_callback(sms->userdata, r, g, b);
            sms->vdp.dirty_cram[i] = false;
        }
    }

    sms->vdp.dirty_cram_min = sms->vdp.dirty_cram_max = 0;
}

static void vdp_update_gg_colours(struct SMS_Core* sms)
{
    for (int i = sms->vdp.dirty_cram_min; i < sms->vdp.dirty_cram_max; i += 2)
    {
        if (sms->vdp.dirty_cram[i])
        {
            // GG colours are in [----BBBBGGGGRRRR] format
            const uint8_t r = (sms->vdp.cram[i + 0] >> 0) & 0xF;
            const uint8_t g = (sms->vdp.cram[i + 0] >> 4) & 0xF;
            const uint8_t b = (sms->vdp.cram[i + 1] >> 0) & 0xF;

            // only 32 colours, 2 bytes per colour!
            sms->vdp.colour[i >> 1] = sms->colour_callback(sms->userdata, r, g, b);
            sms->vdp.dirty_cram[i] = false;
        }
    }

    sms->vdp.dirty_cram_min = sms->vdp.dirty_cram_max = 0;
}

static void vdp_update_sg_colours(struct SMS_Core* sms)
{
    struct Colour { uint8_t r,g,b; };
    // https://www.smspower.org/uploads/Development/sg1000.txt
    static const struct Colour SG_COLOUR_TABLE[] =
    {
        {0x00, 0x00, 0x00}, // 0: transparent
        {0x00, 0x00, 0x00}, // 1: black
        {0x20, 0xC0, 0x20}, // 2: green
        {0x60, 0xE0, 0x60}, // 3: bright green
        {0x20, 0x20, 0xE0}, // 4: blue
        {0x40, 0x60, 0xE0}, // 5: bright blue
        {0xA0, 0x20, 0x20}, // 6: dark red
        {0x40, 0xC0, 0xE0}, // 7: cyan (?)
        {0xE0, 0x20, 0x20}, // 8: red
        {0xE0, 0x60, 0x60}, // 9: bright red
        {0xC0, 0xC0, 0x20}, // 10: yellow
        {0xC0, 0xC0, 0x80}, // 11: bright yellow
        {0x20, 0x80, 0x20}, // 12: dark green
        {0xC0, 0x40, 0xA0}, // 13: pink
        {0xA0, 0xA0, 0xA0}, // 14: gray
        {0xE0, 0xE0, 0xE0}, // 15: white
    };

    // the dirty_* values are only set on romload and
    // loadstate. so we can check this value and if set
    // then update the colours.
    if (sms->vdp.dirty_cram_max == 0)
    {
        return;
    }

    for (int i = 0; i < 16; i++)
    {
        const struct Colour c = SG_COLOUR_TABLE[i];
        sms->vdp.colour[i] = sms->colour_callback(NULL, c.r, c.g, c.b);
    }

    sms->vdp.dirty_cram_min = sms->vdp.dirty_cram_max = 0;
    memset(sms->vdp.dirty_cram, false, sizeof(sms->vdp.dirty_cram));
}

static void vdp_update_palette(struct SMS_Core* sms)
{
    if (sms->colour_callback)
    {
        switch (SMS_get_system_type(sms))
        {
            case SMS_System_SMS:
                vdp_update_sms_colours(sms);
                break;
            case SMS_System_GG:
                vdp_update_gg_colours(sms);
                break;
            case SMS_System_SG1000:
                vdp_update_sg_colours(sms);
                break;
        }
    }
}

void vdp_mark_palette_dirty(struct SMS_Core* sms)
{
    memset(sms->vdp.dirty_cram, true, sizeof(sms->vdp.dirty_cram));
    sms->vdp.dirty_cram_min = 0;

    if (SMS_is_system_type_gg(sms))
    {
        sms->vdp.dirty_cram_max = 64;
    }
    else
    {
        sms->vdp.dirty_cram_max = 32;
    }

    vdp_update_palette(sms);
}

bool vdp_has_interrupt(const struct SMS_Core* sms)
{
    const bool frame_interrupt = VDP.frame_interrupt_pending && vdp_is_vblank_irq_wanted(sms);
    const bool line_interrupt = VDP.line_interrupt_pending && vdp_is_line_irq_wanted(sms);

    return frame_interrupt || line_interrupt;
}

static void vdp_advance_line_counter(struct SMS_Core* sms)
{
    // i don't think sg has line interrupt
    if (!SMS_is_system_type_sg(sms))
    {
        VDP.line_counter--;

        // reloads / fires irq (if enabled) on underflow
        if (VDP.line_counter == 0xFF)
        {
            VDP.line_interrupt_pending = true;
            VDP.line_counter = VDP.registers[0xA];
        }
    }
}

static void vdp_render_frame(struct SMS_Core* sms)
{
    // only render if display is enabled
    if (!vdp_is_display_enabled(sms))
    {
        // on sms/gg, sprite overflow still happens with display disabled
        if (!SMS_is_system_type_sg(sms))
        {
            vdp_parse_sprites(sms);
        }
        return;
    }

    // exit early if we have no pixels (this will break games that need sprite overflow and collision)
    if (!sms->pixels || sms->skip_frame)
    {
        return;
    }

    struct PriorityBuf prio = {0};
    #ifndef SMS_PIXEL_WIDTH
        pixel_width_t scanline[SMS_SCREEN_WIDTH] = {0};
    #else
        pixel_width_t* scanline = (pixel_width_t*)sms->pixels + (VDP.vcount * sms->pitch);
    #endif

    if (SMS_is_system_type_sg(sms))
    {
        // this isn't correct, but it works :)
        if ((VDP.registers[0] & 0x7) == 0)
        {
            vdp_mode1_render_background(sms, scanline);
        }
        else
        {
            vdp_mode2_render_background(sms, scanline);
        }

        vdp_mode1_render_sprites(sms, scanline);
    }
    else // sms / gg render
    {
        vdp_render_background(sms, scanline, &prio);
        vdp_render_sprites(sms, scanline, &prio);
    }

    #ifndef SMS_PIXEL_WIDTH
        write_scanline_to_frame(sms, scanline, VDP.vcount);
    #endif
}

static void vdp_tick(struct SMS_Core* sms)
{
    if (LIKELY(vdp_is_display_active(sms)))
    {
        vdp_update_palette(sms);
        vdp_render_frame(sms);
    }
    else
    {
        VDP.vertical_scroll = VDP.registers[0x9];
    }

    // advance line counter on lines 0-191 and 192
    if (LIKELY(VDP.vcount <= 192))
    {
        vdp_advance_line_counter(sms);
    }
    else
    {
        VDP.line_counter = VDP.registers[0xA];
    }

    if (VDP.vcount == 192) // vblank. TODO: support diff hieght modes
    {
        SMS_skip_frame(sms, false);
        VDP.frame_interrupt_pending = true;

        if (sms->vblank_callback)
        {
            sms->vblank_callback(sms->userdata);
        }
    }

    if (VDP.vcount == 193 && SMS_is_spiderman_int_hack_enabled(sms) && vdp_is_vblank_irq_wanted(sms)) // hack for spiderman, will remove soon
    {
        z80_irq(sms);
    }

    if (VDP.vcount == 218) // see description in types.h for the jump back value
    {
        VDP.vcount_port = 212;
    }

    if (VDP.vcount == 261) // end of frame. TODO: support pal height
    {
        VDP.vcount = 0;
        VDP.vcount_port = 0;
    }
    else
    {
        VDP.vcount++;
        VDP.vcount_port++;
    }
}

void vdp_run(struct SMS_Core* sms, const uint8_t cycles)
{
    VDP.cycles += cycles;

    if (UNLIKELY(VDP.cycles >= NTSC_CYCLES_PER_LINE))
    {
        VDP.cycles -= NTSC_CYCLES_PER_LINE;
        vdp_tick(sms);
    }
}

void vdp_init(struct SMS_Core* sms)
{
    memset(&VDP, 0, sizeof(VDP));
    // update palette
    vdp_mark_palette_dirty(sms);

    // values on starup
    VDP.registers[0x0] = 0x04; // %00000100 (taken from VDPTEST)
    VDP.registers[0x1] = 0x20; // %00100000 (taken from VDPTEST)
    VDP.registers[0x2] = 0xF1; // %11110001 (taken from VDPTEST)
    VDP.registers[0x3] = 0xFF; // %11111111 (taken from VDPTEST)
    VDP.registers[0x4] = 0x03; // %00000011 (taken from VDPTEST)
    VDP.registers[0x5] = 0x81; // %10000001 (taken from VDPTEST)
    VDP.registers[0x6] = 0xFB; // %11111011 (taken from VDPTEST)
    VDP.registers[0x7] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0x8] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0x9] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0xA] = 0xFF; // %11111111 (taken from VDPTEST)
    // vdp registers are write-only, so the the values of 0xB-0xF don't matter

    if (1) // values after bios (todo: optional bios skip)
    {
        VDP.registers[0x0] = 0x36;
        VDP.registers[0x1] = 0x80;
        VDP.registers[0x6] = 0xFB;
    }

    VDP.line_counter = 0xFF;
}
