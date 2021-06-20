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


static inline bool is_vdp_display_active(const struct SMS_Core* sms)
{
    // check the horizontal and vertical positions
    if (VDP.hcount >= NTSC_DISPLAY_HORIZONTAL_START &&
        VDP.hcount <= NTSC_DISPLAY_HORIZONTAL_END &&
        VDP.vcount >= NTSC_DISPLAY_VERTICAL_START &&
        VDP.vcount >= NTSC_DISPLAY_VERTICAL_END
    ){
        return true;
    }

    return false;
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
    
    return v;
}

void vdp_run(struct SMS_Core* sms, uint8_t cycles)
{
    VDP.hcount += cycles * NTSC_HCOUNT_MULT;

    if (is_vdp_display_active(sms))
    {
        // draw screem
    }
    else if (VDP.hcount >= NTSC_HCOUNT_MAX)
    {
        VDP.hcount = 0;
        VDP.vcount++;
        VDP.vcount_port++;

        switch (VDP.vcount)
        {
            case NTSC_VBLANK_START:
                Z80_irq(sms);
                VDP.frame_interrupt_pending = true;
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
