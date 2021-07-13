#include "sms.h"
#include "internal.h"


// [API]
void SMS_set_port_a(struct SMS_Core* sms, enum SMS_PortA pin, bool down)
{
    if (down)
    {
        sms->port.a &= ~pin;
    }
    else
    {
        sms->port.a |= pin;
    }
}

void SMS_set_port_b(struct SMS_Core* sms, enum SMS_PortB pin, bool down)
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
                Z80_nmi(sms);
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
}
