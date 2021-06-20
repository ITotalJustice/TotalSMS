#include "internal.h"


// [API]
void SMS_set_port_a(struct SMS_Core* sms, enum SMS_PortA pin, bool down)
{
    // the pins represet each bit, because of this
    // we can unmask the bit, then reset it.
    sms->port.a &= pin;

    // NOTE: the pins are HI when NOT PRESSED!
    // so when a button is pressed, the pin will be LO!
    sms->port.a |= !down;
}

void SMS_set_port_b(struct SMS_Core* sms, enum SMS_PortB pin, bool down)
{
    // the pins represet each bit, because of this
    // we can unmask the bit, then reset it.
    sms->port.b &= pin;

    // NOTE: the pins are HI when NOT PRESSED!
    // so when a button is pressed, the pin will be LO!
    sms->port.b |= !down;

    if (pin == RESET_BUTTON)
    {
        Z80_nmi(sms);
    }
}
