#include "internal.h"


#define VDP sms->vdp


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
    VDP.hcount += cycles;


    #if 0
    if (VDP.hcount > 241)
    {
        VDP.hcount = 0;
        VDP.vcount++;

        if (VDP.vcount == 191)
        {
            z80_gen_int(&sms->z80, 0);
            Z80_irq(sms);
            VDP.frame_interrupt_pending = true;
        }

        if (VDP.vcount > 241)
        {
            VDP.frame_interrupt_pending = false;
            VDP.vcount = 0;
        }
    }
    
    #else

    if (VDP.hcount > 342)
    {
        VDP.hcount = 0;
        VDP.vcount++;

        if (VDP.vcount == 192)
        {
            Z80_irq(sms);
            VDP.frame_interrupt_pending = true;
        }

        // if (VDP.vcount == 224)
        // {
        //     z80_gen_int(&sms->z80, 0);
        //     Z80_irq(sms);
        //     VDP.frame_interrupt_pending = true;
        // }

        // if (VDP.vcount == 240)
        // {
        //     z80_gen_int(&sms->z80, 0);
        //     Z80_irq(sms);
        //     VDP.frame_interrupt_pending = true;
        // }

        if (VDP.vcount == 262)
        {
            VDP.frame_interrupt_pending = false;
            VDP.vcount = 0;
        }
    }
    #endif
}
