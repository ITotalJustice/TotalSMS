#include "sms.h"
#include "internal.h"


// [API]
void SMS_set_port_a(struct SMS_Core* sms, const enum SMS_PortA pin, const bool down)
{
    if (down)
    {
        sms->port.a &= ~pin;

        // if (is_down && (buttons & GB_BUTTON_DIRECTIONAL))
        // can't have opposing directions pressed at the same time
        {
            if (pin & JOY1_RIGHT_BUTTON)
            {
                sms->port.a |= JOY1_LEFT_BUTTON;
            }
            if (pin & JOY1_LEFT_BUTTON)
            {
                sms->port.a |= JOY1_RIGHT_BUTTON;
            }
            if (pin & JOY1_UP_BUTTON)
            {
                sms->port.a |= JOY1_DOWN_BUTTON;
            }
            if (pin & JOY1_DOWN_BUTTON)
            {
                sms->port.a |= JOY1_UP_BUTTON;
            }
        }
    }
    else
    {
        sms->port.a |= pin;
    }
}

void SMS_set_port_b(struct SMS_Core* sms, const enum SMS_PortB pin, const bool down)
{
    if (pin == PAUSE_BUTTON)
    {
        if (SMS_is_system_type_gg(sms))
        {
            sms->port.gg_regs[0x0] &= ~0x80;

            if (!down)
            {
                sms->port.gg_regs[0x0] |= 0x80;
            }
        }
        else
        {
            if (down)
            {
                z80_nmi(sms);
            }
        }
    }
    else
    {
        if (down)
        {
            sms->port.b &= ~pin;
        }
        else
        {
            sms->port.b |= pin;
        }

    }

    // bit 5 is unused on SMS and GG (used on MD)
    sms->port.b |= 1 << 5;

    if (SMS_is_system_type_gg(sms))
    {
        // GG (and MD) have no reset button so always return 1
        sms->port.b |= 1 << 4;
    }
}
