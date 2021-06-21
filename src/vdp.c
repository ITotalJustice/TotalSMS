#include "internal.h"


#define VDP sms->vdp


enum
{
    NTSC_HCOUNT_MAX = 342,
    PAL_HCOUNT_MAX = 342,

    NTSC_VCOUNT_MAX = 262,
    PAL_VCOUNT_MAX = 312,

    NTSC_FPS = 60,
    PAL_FPS = 50,

    NTSC_CYCLES_PER_FRAME = CPU_CLOCK / NTSC_VCOUNT_MAX / NTSC_FPS, // ~228
    PAL_CYCLES_PER_FRAME = CPU_CLOCK / PAL_VCOUNT_MAX / PAL_FPS, // ~230

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

    NTSC_VBLANK_START = NTSC_VBLANK_TOP + NTSC_BORDER_TOP + NTSC_ACTIVE_DISPLAY_VERTICAL + NTSC_BORDER_BOTTOM,
    NTSC_VBLANK_END = NTSC_VBLANK_TOP,
};

// there are always 342 pixels across, but each scanline is either
// 228 or 230 cycles, so, we need to mult the cpu cycles by a %
// (this will be a decimal, so it cannot be an enum, aka an int)
#define NTSC_HCOUNT_MULT ((float)NTSC_HCOUNT_MAX / (float)NTSC_CYCLES_PER_FRAME)
#define PAL_HCOUNT_MULT ((float)PAL_HCOUNT_MAX / (float)PAL_CYCLES_PER_FRAME)

// to be used for logging
static const char* const VDP_REG_STR[0x10] =
{
    [0x0] = "mode control 0",
    [0x1] = "mode control 1",
    [0x2] = "name table base addr",
    [0x3] = "colour table base addr",
    [0x4] = "background pattern generator addr",
    [0x5] = "sprite attribute table base addr",
    [0x6] = "sprite generator base addr",
    [0x7] = "overscan / background colour",
    [0x8] = "background x scroll",
    [0x9] = "background y scroll",
    [0xA] = "line counter",
    [0xB] = "--UNKNOWN--",
    [0xC] = "--UNKNOWN--",
    [0xD] = "--UNKNOWN--",
    [0xE] = "--UNKNOWN--",
    [0xF] = "--UNKNOWN--",
};

static const char* const VDP_MODE_STR[0x10] =
{
    [0x0] = "graphic 1",
    [0x1] = "text",
    [0x2] = "graphic 2",
    [0x3] = "mode 1+2",
    [0x4] = "multicolour",
    [0x5] = "mode 1+3",
    [0x6] = "mode 2+3",
    [0x7] = "mode 1+2+3",
    [0x8] = "mode 4",
    [0x9] = "invalid text mode",
    [0xA] = "mode 4",
    [0xB] = "invalid text mode",
    [0xC] = "mode 4",
    [0xD] = "invalid text mode",
    [0xE] = "mode 4",
    [0xF] = "invalid text mode",
};

static inline bool vdp_is_display_active(const struct SMS_Core* sms)
{
    if (VDP.vcount >= NTSC_DISPLAY_VERTICAL_START &&
        VDP.vcount <= NTSC_DISPLAY_VERTICAL_END
    ){
        return true;
    }

    return false;
}

static inline uint8_t vdp_get_display_mode(const struct SMS_Core* sms)
{
    uint8_t v = 0;
    v |= (IS_BIT_SET(VDP.registers[0], 2)) << 3;
    v |= (IS_BIT_SET(VDP.registers[0], 1)) << 1;
    v |= (IS_BIT_SET(VDP.registers[1], 4)) << 0;
    v |= (IS_BIT_SET(VDP.registers[1], 3)) << 2;
    return v;
}

static inline bool vdp_is_line_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 4);
}

static inline bool vdp_is_vblank_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 5);
}

static uint16_t vdp_get_nametable_base_addr(const struct SMS_Core* sms)
{
    return ((VDP.registers[0x2] >> 1) & 0x7) << 11;
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
    // NOTE: this is a mode-4 only impl!
    // the regs have different meanings based on the mode, which is out
    // of scope for this emu.
    VDP.registers[addr & 0xF] = value;

    (void)VDP_MODE_STR;
    (void)VDP_REG_STR;
}

// this is just for testing.
// converts colour to bgr555.
static inline uint16_t convert_colour(uint8_t v)
{
    const uint8_t r = (v >> 0) & 0x3;
    const uint8_t g = (v >> 2) & 0x3;
    const uint8_t b = (v >> 4) & 0x3;

    return (r << 13) | (g << 8) | (b << 3);
}

static inline void vdp_render_background(struct SMS_Core* sms)
{
    const uint16_t pixely = VDP.vcount;
    const uint16_t pixelx = NTSC_DISPLAY_HORIZONTAL_START;
    
    const uint8_t line = VDP.vcount - NTSC_DISPLAY_VERTICAL_START;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;

    const uint8_t starting_col = (32 - (VDP.registers[0x8] >> 3)) & 31;
    const uint8_t fine_scrollx = VDP.registers[0x8] & 0x7;

    const uint8_t starting_row = VDP.vertical_scroll >> 3;
    const uint8_t fine_scrolly = VDP.vertical_scroll & 0x7;

    (void)fine_scrolly; // todo:

    const uint16_t nametable_base_addr = vdp_get_nametable_base_addr(sms);
    // the 28 depends on the mode(?) of vdp, it can also be 32
    const uint16_t vertical_offset = ((row + starting_row) % 28) * 64;

    for (uint8_t col = 0; col < 32; ++col)
    {
        const uint16_t horizontal_offset = ((starting_col + col) & 31) * 2;
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
            pattern_index += (7 - fine_line) * 4;
        }
        else
        {
            pattern_index += fine_line * 4;
        }
        
        // todo: keep an array of priority bits (like the gb)
        // and pass it to the sprite render function
        (void)priority;

        const uint8_t tile_def0 = VDP.vram[pattern_index + 0];
        const uint8_t tile_def1 = VDP.vram[pattern_index + 1];
        const uint8_t tile_def2 = VDP.vram[pattern_index + 2];
        const uint8_t tile_def3 = VDP.vram[pattern_index + 3];

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t x_index = ((col * 8) + x + fine_scrollx) & 0xFF;

            const uint8_t bit = horizontal_flip ? x : 7 - x;

            // if set, we load from the tile index instead!
            uint8_t palette_index = palette_select ? 16 : 0;

            palette_index |= IS_BIT_SET(tile_def0, bit) << 0;
            palette_index |= IS_BIT_SET(tile_def1, bit) << 1;
            palette_index |= IS_BIT_SET(tile_def2, bit) << 2;
            palette_index |= IS_BIT_SET(tile_def3, bit) << 3;

            VDP.pixels[pixely][pixelx + x_index] = convert_colour(VDP.cram[palette_index]);
        }
    }
}

void vdp_run(struct SMS_Core* sms, uint8_t cycles)
{
    VDP.hcount += cycles * NTSC_HCOUNT_MULT;

    if (VDP.hcount >= NTSC_HCOUNT_MAX)
    {
        if (vdp_is_display_active(sms))
        {
            VDP.line_counter--;

            if (VDP.line_counter == 0xFF)
            {
                if (vdp_is_line_irq_wanted(sms))
                {
                    Z80_irq(sms);
                }

                VDP.line_counter = VDP.registers[0xA];
            }

            vdp_render_background(sms);
        }
        else
        {
            VDP.vertical_scroll = VDP.registers[0x9];
            VDP.line_counter = VDP.registers[0xA];
        }

        // -= because we don't want to drift too far
        // only the lightgun reads hcount so accuracy of this isn't important!
        VDP.hcount -= NTSC_HCOUNT_MAX;
        VDP.vcount++;
        VDP.vcount_port++;

        switch (VDP.vcount)
        {
            case NTSC_VBLANK_START:
                if (vdp_is_vblank_irq_wanted(sms))
                {
                    Z80_irq(sms);                    
                }
                VDP.frame_interrupt_pending = true;

                if (VDP.vblank_callback)
                {
                    VDP.vblank_callback(VDP.vblank_callback_user);
                }
                break;

            case NTSC_VBLANK_END:
                VDP.frame_interrupt_pending = false;
                break;

            case 218: // see description in types.h for the jump back value
                VDP.vcount_port = 213;
                break;

            case NTSC_VCOUNT_MAX:
                VDP.vcount = 0;
                VDP.vcount_port = 0;
                break;
        }
    }
}
