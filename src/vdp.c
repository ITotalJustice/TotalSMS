#include "sms.h"
#include "internal.h"
#include <assert.h>


#define VDP sms->vdp


enum
{
    NTSC_HCOUNT_MAX = 342,
    NTSC_VCOUNT_MAX = 262,

    NTSC_FPS = 60,

    NTSC_CYCLES_PER_LINE = CPU_CLOCK / NTSC_VCOUNT_MAX / NTSC_FPS, // ~228

    NTSC_HBLANK_LEFT = 29,
    NTSC_HBLANK_RIGHT = 29,

    NTSC_BORDER_TOP = 27,
    NTSC_BORDER_BOTTOM = 24,

    NTSC_BORDER_LEFT = 13,
    NTSC_BORDER_RIGHT = 15,

    NTSC_ACTIVE_DISPLAY_HORIZONTAL = 256,
    NTSC_ACTIVE_DISPLAY_VERTICAL = 192,

    NTSC_DISPLAY_VERTICAL_START = NTSC_BORDER_TOP,
    NTSC_DISPLAY_VERTICAL_END = NTSC_DISPLAY_VERTICAL_START + NTSC_ACTIVE_DISPLAY_VERTICAL,

    NTSC_DISPLAY_HORIZONTAL_START = NTSC_BORDER_LEFT,
    NTSC_DISPLAY_HORIZONTAL_END = NTSC_DISPLAY_HORIZONTAL_START + NTSC_ACTIVE_DISPLAY_HORIZONTAL,

    SPRITE_EOF = 208,
};


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

struct VDP_region
{
    uint16_t startx;
    uint16_t endx;
    uint16_t pixelx;
    uint16_t pixely;
};

static inline struct VDP_region vdp_get_region(const struct SMS_Core* sms)
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
            .endx = NTSC_ACTIVE_DISPLAY_HORIZONTAL,
            .pixelx = NTSC_DISPLAY_HORIZONTAL_START,
            .pixely = VDP.vcount + NTSC_DISPLAY_VERTICAL_START,
        };
    }
}

static inline bool vdp_is_display_active(const struct SMS_Core* sms)
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

// fills the entire pixel array with overscan colour.
// this is only ever called once a frame
static void vdp_render_overscan(struct SMS_Core* sms)
{
    const uint32_t overscan_col = VDP.colour[16 + vdp_get_overscan_colour(sms)];

    #define RENDER_OVERSCAN(type) do { \
        for (int y = 0; y < SMS_SCREEN_HEIGHT; ++y) \
        { \
            uint##type##_t* pixels = ((uint##type##_t*)sms->pixels) + sms->pixels_stride * y; \
            for (int x = 0; x < SMS_SCREEN_WIDTH; ++x) \
            { \
                pixels[x] = overscan_col; \
            } \
        } \
    } while (0)

    #ifdef SMS_BPP
        #if SMS_BPP == 8
            RENDER_OVERSCAN(8);
        #elif SMS_BPP == 15 || SMS_BPP == 16
            RENDER_OVERSCAN(16);
        #elif SMS_BPP == 24 || SMS_BPP == 32
            RENDER_OVERSCAN(32);
        #endif
    #else
        switch (sms->bpp)
        {
            case 8:
                RENDER_OVERSCAN(8);
                break;

            case 15:
            case 16:
                RENDER_OVERSCAN(16);
                break;

            case 24:
            case 32:
                RENDER_OVERSCAN(32);
                break;
        }
    #endif
    #undef RENDER_OVERSCAN
}

static FORCE_INLINE void vdp_write_pixel(struct SMS_Core* sms, uint32_t c, uint16_t x, uint16_t y)
{
    // the bpp can be defined at build time as a slight optimisation
    #ifdef SMS_BPP
        assert(sms->bpp == SMS_BPP && "bpp missmatch with build option!");

        #if SMS_BPP == 8
            ((uint8_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
        #elif SMS_BPP == 15 || SMS_BPP == 16
            ((uint16_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
        #elif SMS_BPP == 24 || SMS_BPP == 32
            ((uint32_t*)sms->pixels)[sms->pixels_stride * y + x] = c;
        #else
            #error "invalid SMS_BPP! use 8, 15,16, 24,32"
        #endif
    #else
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

            default:
                assert(0 && "bpp invalid option!");
                break;
        }
    #endif
}

uint8_t vdp_status_flag_read(struct SMS_Core* sms)
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

void vdp_io_write(struct SMS_Core* sms, uint8_t addr, uint8_t value)
{
    switch (addr & 0xF)
    {
        case 0x1:
            assert(IS_BIT_SET(value, 3) == 0 && "240 height mode set");
            assert(IS_BIT_SET(value, 4) == 0 && "224 height mode set");
            break;

        case 0x3:
            assert(value == 0xFF && "colour table bits not all set");
            break;

        case 0x4:
            assert((value & 0xF) == 0xF && "Background Pattern Generator Base Address");
            break;

        case 0x09:
            if (VDP.vcount >= 192)
            {
                VDP.vertical_scroll = value;
            }
            break;

        case 0xA:
            if (VDP.vcount >= 192)
            {
                VDP.line_counter = value;
            }
            break;
    }

    VDP.registers[addr & 0xF] = value;
}

// same as i used in dmg / gbc rendering for gb
struct PriorityBuf
{
    bool array[NTSC_ACTIVE_DISPLAY_HORIZONTAL];
};

static void vdp_render_background(struct SMS_Core* sms, struct PriorityBuf* prio)
{
    const struct VDP_region region = vdp_get_region(sms);
    
    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;

    const uint8_t starting_col = (32 - (VDP.registers[0x8] >> 3)) & 31;
    uint8_t fine_scrollx = VDP.registers[0x8] & 0x7;

    const uint8_t starting_row = VDP.vertical_scroll >> 3;
    const uint8_t fine_scrolly = VDP.vertical_scroll & 0x7;

    const uint16_t nametable_base_addr = vdp_get_nametable_base_addr(sms);

    for (uint8_t col = 0; col < 32; ++col)
    {
        uint16_t horizontal_offset = 0;

        // check if horizontal scrolling should be disabled
        // doesn't work in GG mode as the screen centered
        if (IS_BIT_SET(VDP.registers[0x0], 6) && line < 16)
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

        uint8_t check_col = 0;

        // if set, we use the internal col counter, else, the starting_col
        // can be moved outside of the loop, though i don't think it impacts
        // performance at all.
        if (IS_BIT_SET(VDP.registers[0x1], 7))
        {
            check_col = col;
        }
        else
        {
            check_col = (col + starting_col) & 31;
        }

        // check if vertical scrolling should be disabled
        if (IS_BIT_SET(VDP.registers[0x0], 7) && check_col >= 24)
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

            if (x_index >= region.endx)
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

            // if startx == 8, then first column is overscan colour!
            if (x_index < region.startx)
            {
                // otherwise, this is in GG mode and skipping the first few cols
                if (region.startx != 8)
                {
                    continue;
                }

                palette_index = 16 + vdp_get_overscan_colour(sms);
            }

            // used when sprite rendering, will skip if prio set and not pal0
            prio->array[x_index] = priority && palette_index != 0;

            vdp_write_pixel(sms, VDP.colour[palette_index], region.pixelx + x_index, region.pixely);
        }
    }
}

void SMS_get_pixel_region(const struct SMS_Core* sms, uint16_t* x, uint16_t* y, uint16_t* w, uint16_t* h)
{
    if (SMS_is_overscan_enabled(sms))
    {
        *x = 0;
        *y = 0;
        *w = SMS_SCREEN_WIDTH;
        *h = SMS_SCREEN_HEIGHT;
    }
    else
    {
        // todo: support different height modes
        if (SMS_is_system_type_gg(sms))
        {
            *x = 62;
            *y = 51;
            *w = 160;
            *h = 144;
        }
        else
        {
            *x = NTSC_DISPLAY_HORIZONTAL_START;
            *y = NTSC_DISPLAY_VERTICAL_START;
            *w = NTSC_ACTIVE_DISPLAY_HORIZONTAL;
            *h = NTSC_ACTIVE_DISPLAY_VERTICAL;
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
    assert(IS_BIT_SET(VDP.registers[0x5], 0) && "needs lower index for oam");
    assert((VDP.registers[0x6] & 0x3) == 0x3 && "Sprite Pattern Generator Base Address");

    struct SpriteEntries sprites = {0};

    const uint8_t line = VDP.vcount;
    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
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
    const struct VDP_region region = vdp_get_region(sms);
    
    const uint8_t line = VDP.vcount;
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
            if (x_index < region.startx)
            {
                continue;
            }

            if (x_index >= region.endx)
            {
                break;
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

            // skip if we already rendered a sprite!
            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // keep track of this sprite already being rendered
            drawn_sprites[x_index] = true;

            // sprite cram index is the upper 16-bytes!
            vdp_write_pixel(sms, VDP.colour[palette_index + 16], region.pixelx + x_index, region.pixely);
        }
    }
}

static void vdp_update_sms_colours(struct SMS_Core* sms)
{
    const uint8_t overscan_index = 16 + vdp_get_overscan_colour(sms);

    for (size_t i = 0; i < 32; ++i)
    {
        if (sms->vdp.dirty_cram[i])
        {
            const uint8_t r = (sms->vdp.cram[i] >> 0) & 0x3;
            const uint8_t g = (sms->vdp.cram[i] >> 2) & 0x3;
            const uint8_t b = (sms->vdp.cram[i] >> 4) & 0x3;

            sms->vdp.colour[i] = sms->colour_callback(sms->colour_callback_user, r, g, b);
            sms->vdp.dirty_cram[i] = false;

            if (i == overscan_index)
            {
                VDP.dirty_overscan_colour = true;
            }
        }
    }
}

static void vdp_update_gg_colours(struct SMS_Core* sms)
{
    const uint8_t overscan_index = 16 + vdp_get_overscan_colour(sms);

    for (size_t i = 0; i < 64; i += 2)
    {
        if (sms->vdp.dirty_cram[i])
        {
            // GG colours are in [----BBBBGGGGRRRR] format
            const uint8_t r = (sms->vdp.cram[i + 0] >> 0) & 0xF;
            const uint8_t g = (sms->vdp.cram[i + 0] >> 4) & 0xF;
            const uint8_t b = (sms->vdp.cram[i + 1] >> 0) & 0xF;

            // only 32 colours, 2 bytes per colour!
            sms->vdp.colour[i >> 1] = sms->colour_callback(sms->colour_callback_user, r, g, b);
            sms->vdp.dirty_cram[i] = false;

            if ((i >> 1) == overscan_index)
            {
                VDP.dirty_overscan_colour = true;
            }
        }
    }
}

bool vdp_has_interrupt(const struct SMS_Core* sms)
{
    if (VDP.frame_interrupt_pending && vdp_is_vblank_irq_wanted(sms))
    {
        return true;
    }

    if (VDP.line_interrupt_pending && vdp_is_line_irq_wanted(sms))
    {
        return true;
    }

    return false;
}

void vdp_run(struct SMS_Core* sms, uint8_t cycles)
{
    VDP.cycles += cycles;

    if (VDP.cycles >= NTSC_CYCLES_PER_LINE)
    {
        if (VDP.vcount < 192)
        {
            if (sms->colour_callback)
            {
                if (SMS_is_system_type_gg(sms))
                {
                    vdp_update_gg_colours(sms);
                }
                else
                {
                    vdp_update_sms_colours(sms);
                }
            }

            VDP.line_counter--;

            // reloads / fires irq (if enabled) on underflow
            if (VDP.line_counter == 0xFF)
            {
                VDP.line_interrupt_pending = true;
                VDP.line_counter = VDP.registers[0xA];
            }

            // only render if enabled and we have pixels
            if (vdp_is_display_enabled(sms) && sms->pixels)
            {
                if (SMS_is_overscan_enabled(sms) && VDP.dirty_overscan_colour && VDP.vcount == 0)
                {
                    vdp_render_overscan(sms);
                    VDP.dirty_overscan_colour = false;

                    SMS_log("rendering overscan!\n");
                }

                if (vdp_is_display_active(sms))
                {
                    struct PriorityBuf prio = {0};

                    vdp_render_background(sms, &prio);
                    vdp_render_sprites(sms, &prio);
                }
            }
        }

        // this can be optimised to add cycles for line 261 (15k+ cycles)
        VDP.cycles -= NTSC_CYCLES_PER_LINE;
        VDP.vcount++;
        VDP.vcount_port++;

        switch (VDP.vcount)
        {
            case 193:
                VDP.frame_interrupt_pending = true;
                VDP.vertical_scroll = VDP.registers[0x9];
                VDP.line_counter = VDP.registers[0xA];

                if (sms->vblank_callback)
                {
                    sms->vblank_callback(sms->vblank_callback_user);
                }
                break;

            // see description in types.h for the jump back value
            case 219:
                VDP.vcount_port = 213;
                break;

            case 262:
                VDP.vcount = 0;
                VDP.vcount_port = 0;
                break;
        }
    }
}
