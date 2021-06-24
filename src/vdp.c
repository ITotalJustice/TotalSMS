#include "internal.h"
#include <assert.h>


#define VDP sms->vdp


enum
{
    NTSC_HCOUNT_MAX = 342,
    PAL_HCOUNT_MAX = 342,

    NTSC_VCOUNT_MAX = 262,
    PAL_VCOUNT_MAX = 312,

    NTSC_FPS = 60,
    PAL_FPS = 50,

    NTSC_CYCLES_PER_LINE = CPU_CLOCK / NTSC_VCOUNT_MAX / NTSC_FPS, // ~228
    PAL_CYCLES_PER_LINE = CPU_CLOCK / PAL_VCOUNT_MAX / PAL_FPS, // ~230

    NTSC_VBLANK_TOP = 16,
    NTSC_VBLANK_BOTTOM = 3,

    NTSC_HBLANK_LEFT = 29,
    NTSC_HBLANK_RIGHT = 29,

    NTSC_BORDER_TOP = 27,
    NTSC_BORDER_BOTTOM = 24,

    NTSC_BORDER_LEFT = 13,
    NTSC_BORDER_RIGHT = 15,

    NTSC_ACTIVE_DISPLAY_HORIZONTAL = 256,
    NTSC_ACTIVE_DISPLAY_VERTICAL = 192,

    NTSC_DISPLAY_VERTICAL_START = NTSC_VBLANK_TOP + NTSC_BORDER_TOP,
    NTSC_DISPLAY_VERTICAL_END = NTSC_DISPLAY_VERTICAL_START + NTSC_ACTIVE_DISPLAY_VERTICAL,

    NTSC_DISPLAY_HORIZONTAL_START = NTSC_HBLANK_LEFT + NTSC_BORDER_LEFT,
    NTSC_DISPLAY_HORIZONTAL_END = NTSC_DISPLAY_HORIZONTAL_START + NTSC_ACTIVE_DISPLAY_HORIZONTAL,

    NTSC_VBLANK_START = NTSC_DISPLAY_VERTICAL_END + NTSC_BORDER_BOTTOM,
    NTSC_VBLANK_END = NTSC_VBLANK_TOP,

    SPRITE_EOF = 208,
};

// there are always 342 pixels across, but each scanline is either
// 228 or 230 cycles, so, we need to mult the cpu cycles by a %
// (this will be a decimal, so it cannot be an enum, aka an int)
#define NTSC_HCOUNT_MULT ((float)NTSC_HCOUNT_MAX / (float)NTSC_CYCLES_PER_LINE)
#define PAL_HCOUNT_MULT ((float)PAL_HCOUNT_MAX / (float)PAL_CYCLES_PER_LINE)


static bool vdp_is_display_active(const struct SMS_Core* sms)
{
    return VDP.vcount >= NTSC_DISPLAY_VERTICAL_START && VDP.vcount < NTSC_DISPLAY_VERTICAL_END;
}

static bool vdp_is_line_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 4);
}

static bool vdp_is_vblank_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 5);
}

static uint16_t vdp_get_nametable_base_addr(const struct SMS_Core* sms)
{
    return ((VDP.registers[0x2] >> 1) & 0x7) << 11;
}

static uint16_t vdp_get_sprite_attribute_base_addr(const struct SMS_Core* sms)
{
    return ((VDP.registers[0x5] >> 1) & 0x3F) << 8;
}

static bool vdp_get_sprite_pattern_select(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x6], 2);
}

static bool vdp_is_display_enabled(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 6);
}

static uint8_t vdp_get_sprite_height(const struct SMS_Core* sms)
{
    const bool doubled_sprites = IS_BIT_SET(VDP.registers[0x1], 0);
    const uint8_t sprite_size = IS_BIT_SET(VDP.registers[0x1], 1) ? 16 : 8;

    return sprite_size << doubled_sprites;
}

static uint8_t vdp_get_overscan_colour(const struct SMS_Core* sms)
{
    return VDP.registers[0x7] & 0xF;
}

static FORCE_INLINE void vdp_write_pixel(struct SMS_Core* sms, uint32_t c, uint16_t x, uint16_t y)
{
    switch (sms->bpp)
    {
        case 8:
            ((uint8_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
            break;

        case 15:
        case 16:
            ((uint16_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
            break;

        case 24:
        case 32:
            ((uint32_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
            break;
    }
}

uint8_t vdp_status_flag_read(struct SMS_Core* sms)
{
    uint8_t v = 0;

    v |= VDP.frame_interrupt_pending << 7;
    v |= VDP.sprite_overflow << 6;
    v |= VDP.sprite_collision << 5;

    // these are reset on read
    VDP.frame_interrupt_pending = false;
    VDP.sprite_overflow = false;
    VDP.sprite_collision = false;
    
    return v;
}

void vdp_io_write(struct SMS_Core* sms, uint8_t addr, uint8_t value)
{
    VDP.registers[addr & 0xF] = value;
}

// same as i used in dmg / gbc rendering for gb
struct PriorityBuf
{
    bool array[NTSC_ACTIVE_DISPLAY_HORIZONTAL];
};

static inline void vdp_render_overscan(struct SMS_Core* sms, uint16_t from, uint16_t until)
{
    const uint32_t overscan_col = VDP.colour[16 + vdp_get_overscan_colour(sms)];

    for (uint16_t i = 0; i < until; ++i)
    {
        vdp_write_pixel(sms, overscan_col, from + i - NTSC_HBLANK_LEFT, VDP.vcount - NTSC_VBLANK_TOP);
    }
}

static void vdp_render_background(struct SMS_Core* sms, struct PriorityBuf* prio)
{
    const uint16_t pixely = VDP.vcount - NTSC_VBLANK_TOP;
    const uint16_t pixelx = NTSC_DISPLAY_HORIZONTAL_START - NTSC_HBLANK_LEFT;
    
    const uint8_t line = VDP.vcount - NTSC_DISPLAY_VERTICAL_START;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;

    const uint8_t starting_col = (32 - (VDP.registers[0x8] >> 3)) & 31;
    uint8_t fine_scrollx = VDP.registers[0x8] & 0x7;

    const uint8_t starting_row = VDP.vertical_scroll >> 3;
    const uint8_t fine_scrolly = VDP.vertical_scroll & 0x7;

    const uint16_t nametable_base_addr = vdp_get_nametable_base_addr(sms);

    // render overscan left
    vdp_render_overscan(sms, NTSC_HBLANK_LEFT, NTSC_BORDER_LEFT + 8);
    // render overscan right
    vdp_render_overscan(sms, NTSC_DISPLAY_HORIZONTAL_END, NTSC_BORDER_RIGHT);


    for (uint8_t col = 0; col < 32; ++col)
    {
        uint16_t horizontal_offset = 0;

        // check if horizontal scrolling should be disabled
        if (IS_BIT_SET(VDP.registers[0x0], 6) && line <= 15)
        {
            horizontal_offset = col * 2;
            fine_scrollx = 0;
        }
        else
        {
            horizontal_offset = ((starting_col + col) & 31) * 2;
            fine_scrollx = VDP.registers[0x8] & 0x7;
        }

        uint16_t vertical_offset = 0;
        uint8_t palette_index_offset = 0;

        // check if vertical scrolling should be disabled
        if (IS_BIT_SET(VDP.registers[0x0], 7) && col >= 24)
        {
            vertical_offset = (row % 28) * 64;
            palette_index_offset = fine_line;
        }
        else
        {
            // we need to check if we cross the next row
            const bool next_row = (fine_line + fine_scrolly) > 7;

            vertical_offset = ((row + starting_row + next_row) % 28) * 64;
            palette_index_offset = (fine_line + fine_scrolly) & 0x7;
        }
        
        const uint16_t nametable_addr = nametable_base_addr + vertical_offset + horizontal_offset;

        uint16_t tile = 0;

        tile |= VDP.vram[nametable_addr + 0] << 0;
        tile |= VDP.vram[nametable_addr + 1] << 8;

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

        const uint8_t bit_plane0 = VDP.vram[pattern_index + 0];
        const uint8_t bit_plane1 = VDP.vram[pattern_index + 1];
        const uint8_t bit_plane2 = VDP.vram[pattern_index + 2];
        const uint8_t bit_plane3 = VDP.vram[pattern_index + 3];

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint16_t x_index = (col * 8) + x + fine_scrollx;

            if (x_index < 8)
            {
                continue;
            }

            if (x_index >= NTSC_ACTIVE_DISPLAY_HORIZONTAL)
            {
                break;
            }

            const uint8_t bit = horizontal_flip ? x : 7 - x;

            // if set, we load from the tile index instead!
            uint8_t palette_index = palette_select ? 16 : 0;

            palette_index |= IS_BIT_SET(bit_plane0, bit) << 0;
            palette_index |= IS_BIT_SET(bit_plane1, bit) << 1;
            palette_index |= IS_BIT_SET(bit_plane2, bit) << 2;
            palette_index |= IS_BIT_SET(bit_plane3, bit) << 3;

            // used when sprite rendering, will skip if prio set and not pal0
            prio->array[x_index] = priority && palette_index != 0;

            vdp_write_pixel(sms, VDP.colour[palette_index], pixelx + x_index, pixely);
        }
    }
}

struct SpriteEntry
{
    uint8_t y;
    uint8_t xn_index;
};

struct SpriteEntries
{
    struct SpriteEntry entry[8];
    uint8_t count;
};

static struct SpriteEntries vdp_parse_sprites(struct SMS_Core* sms)
{
    assert(IS_BIT_SET(VDP.registers[0x5], 0) && "needs lower index for oam");

    struct SpriteEntries sprites = {0};

    const uint8_t line = VDP.vcount - NTSC_DISPLAY_VERTICAL_START;
    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
    const uint8_t sprite_size = vdp_get_sprite_height(sms);

    for (uint8_t i = 0; i < 64; ++i)
    {
        // u16 as the sprite value could be 0xFF, which would overflow :)
        uint16_t y = VDP.vram[sprite_attribute_base_addr + i] + 1;
    
        // special number used to stop sprite parsing!
        if (y == SPRITE_EOF + 1)
        {
            break;
        }

        if (line >= y && line < (y + sprite_size))
        {
            if (sprites.count < 8)
            {
                sprites.entry[sprites.count].y = y;
                // xn values start at + 0x80 in the SAT
                sprites.entry[sprites.count].xn_index = 0x80 + (i * 2);
                sprites.count++;
            }

            // if we have filled the sprite array, we need to keep checking further
            // entries just in case another sprite falls on the same line, in which
            // case, the sprite overflow flag is set for stat.
            else
            {
                VDP.sprite_overflow = true;
                break;
            }
        }
    }

    return sprites;
}

static void vdp_render_sprites(struct SMS_Core* sms, const struct PriorityBuf* prio)
{
    const uint16_t pixely = VDP.vcount - NTSC_VBLANK_TOP;
    const uint16_t pixelx = NTSC_DISPLAY_HORIZONTAL_START - NTSC_HBLANK_LEFT;
    
    const uint8_t line = VDP.vcount - NTSC_DISPLAY_VERTICAL_START;
    const uint16_t attr_addr = vdp_get_sprite_attribute_base_addr(sms);
    // if set, we fetch patterns from upper table
    const uint16_t pattern_select = vdp_get_sprite_pattern_select(sms) ? 256 : 0;
    // if set, sprites start 8 to the left
    const int8_t sprite_x_offset = IS_BIT_SET(VDP.registers[0x0], 3) ? -8 : 0;

    const struct SpriteEntries sprites = vdp_parse_sprites(sms);

    bool drawn_sprites[NTSC_ACTIVE_DISPLAY_HORIZONTAL] = {0};

    for (uint8_t i = 0; i < sprites.count; ++i)
    {
        const struct SpriteEntry* sprite = &sprites.entry[i];

        // signed because the sprite can be negative if -8!
        const int16_t sprite_x = VDP.vram[attr_addr + sprite->xn_index + 0] + sprite_x_offset;

        uint16_t pattern_index = (VDP.vram[attr_addr + sprite->xn_index + 1] + pattern_select);

        // docs state that is bit1 of reg1 is set (should always), bit0 is ignored
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

        const uint8_t bit_plane0 = VDP.vram[pattern_index + 0];
        const uint8_t bit_plane1 = VDP.vram[pattern_index + 1];
        const uint8_t bit_plane2 = VDP.vram[pattern_index + 2];
        const uint8_t bit_plane3 = VDP.vram[pattern_index + 3];

        for (uint8_t x = 0; x < 8; ++x)
        {
            const int16_t x_index = x + sprite_x;

            // skip if column0 or offscreen
            if (x_index < 8 || x_index >= NTSC_ACTIVE_DISPLAY_HORIZONTAL)
            {
                continue;
            }

            // skip if we already rendered a sprite!
            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // skip is bg has priority
            if (prio->array[x_index])
            {
                continue;
            }

            // if set, we load from the tile index instead!
            uint8_t palette_index = 0;

            palette_index |= IS_BIT_SET(bit_plane0, 7 - x) << 0;
            palette_index |= IS_BIT_SET(bit_plane1, 7 - x) << 1;
            palette_index |= IS_BIT_SET(bit_plane2, 7 - x) << 2;
            palette_index |= IS_BIT_SET(bit_plane3, 7 - x) << 3;

            // for sprites, pal0 is transparent
            if (palette_index == 0)
            {
                continue;
            }

            // keep track of this sprite already being rendered
            drawn_sprites[x_index] = true;

            // sprite cram index is the upper 16-bytes!
            vdp_write_pixel(sms, VDP.colour[palette_index + 16], pixelx + x_index, pixely);
        }
    }
}

static bool vdp_is_vertcal_border(const struct SMS_Core* sms)
{
    return (VDP.vcount >= NTSC_VBLANK_TOP && VDP.vcount < (NTSC_VBLANK_TOP + NTSC_BORDER_TOP)) || (VDP.vcount >= NTSC_DISPLAY_VERTICAL_END && VDP.vcount < (NTSC_DISPLAY_VERTICAL_END + NTSC_BORDER_BOTTOM));
}

void vdp_run(struct SMS_Core* sms, uint8_t cycles)
{
    VDP.hcount += cycles * NTSC_HCOUNT_MULT;

    if (VDP.hcount >= NTSC_HCOUNT_MAX)
    {
        if (vdp_is_vertcal_border(sms) && vdp_is_display_enabled(sms) && sms->pixels)
        {
            vdp_render_overscan(sms, NTSC_HBLANK_LEFT, NTSC_ACTIVE_DISPLAY_HORIZONTAL + NTSC_BORDER_RIGHT + NTSC_BORDER_LEFT);
        }

        if (vdp_is_display_active(sms))
        {
            VDP.line_counter--;

            // reloads / fires irq (if enabled) on underflow
            if (VDP.line_counter == 0xFF)
            {
                if (vdp_is_line_irq_wanted(sms))
                {
                    Z80_irq(sms);
                }

                VDP.line_counter = VDP.registers[0xA];
            }

            // only render if enabled and we have pixels
            if (vdp_is_display_enabled(sms) && sms->pixels)
            {
                struct PriorityBuf prio = {0};

                vdp_render_background(sms, &prio);
                vdp_render_sprites(sms, &prio);
            }
        }
        else
        {
            VDP.vertical_scroll = VDP.registers[0x9];
            VDP.line_counter = VDP.registers[0xA];
        }

        switch (VDP.vcount)
        {
            case NTSC_VBLANK_START:
                if (vdp_is_vblank_irq_wanted(sms))
                {
                    Z80_irq(sms);                    
                }

                VDP.frame_interrupt_pending = true;

                if (sms->vblank_callback)
                {
                    sms->vblank_callback(sms->vblank_callback_user);
                }
                break;

            case 218: // see description in types.h for the jump back value
                VDP.vcount_port = 213;
                break;
        }

        if (VDP.vcount == NTSC_VCOUNT_MAX)
        {
            VDP.vcount = 0;
            VDP.vcount_port = 0;
        }
        else
        {
            VDP.vcount++;
            VDP.vcount_port++;
        }

        VDP.hcount -= NTSC_HCOUNT_MAX;
    }
}
