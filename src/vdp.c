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
};

// there are always 342 pixels across, but each scanline is either
// 228 or 230 cycles, so, we need to mult the cpu cycles by a %
// (this will be a decimal, so it cannot be an enum, aka an int)
#define NTSC_HCOUNT_MULT ((float)NTSC_HCOUNT_MAX / (float)NTSC_CYCLES_PER_FRAME)
#define PAL_HCOUNT_MULT ((float)PAL_HCOUNT_MAX / (float)PAL_CYCLES_PER_FRAME)


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

    if (VDP.hcount >= NTSC_HCOUNT_MAX)
    {
        VDP.hcount = 0;
        VDP.vcount++;

        if (VDP.vcount == 192)
        {
            Z80_irq(sms);
            VDP.frame_interrupt_pending = true;
        }

        if (VDP.vcount == NTSC_VCOUNT_MAX)
        {
            VDP.frame_interrupt_pending = false;
            VDP.vcount = 0;
        }
    }
}
