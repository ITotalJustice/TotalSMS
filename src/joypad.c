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
   if (down)
    {
        sms->port.b &= ~pin;
    }
    else
    {
        sms->port.b |= pin;
    }

    if (pin == RESET_BUTTON && down)
    {
        Z80_nmi(sms);
    }
}
