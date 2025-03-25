#include "sms.h"
#include "sms_internal.h"

enum SMS_PortA
{
    JOY1_UP_BUTTON      = 1 << 0,
    JOY1_DOWN_BUTTON    = 1 << 1,
    JOY1_LEFT_BUTTON    = 1 << 2,
    JOY1_RIGHT_BUTTON   = 1 << 3,
    JOY1_A_BUTTON       = 1 << 4,
    JOY1_B_BUTTON       = 1 << 5,
    JOY2_UP_BUTTON      = 1 << 6,
    JOY2_DOWN_BUTTON    = 1 << 7,
};

enum SMS_PortB
{
    JOY2_LEFT_BUTTON    = 1 << 0,
    JOY2_RIGHT_BUTTON   = 1 << 1,
    JOY2_A_BUTTON       = 1 << 2,
    JOY2_B_BUTTON       = 1 << 3,
    RESET_BUTTON        = 1 << 4,
    PAUSE_BUTTON        = 1 << 5,
};

static bool is_button_down(uint8_t pin, uint8_t button)
{
    return !(pin & button);
}

static void joypad_set_port_a(struct SMS_Core* sms, const uint8_t pin, const bool down)
{
    if (down)
    {
        sms->port.a &= ~pin;

        // can't have opposing directions pressed at the same time
        if (pin & JOY1_RIGHT_BUTTON)
        {
            sms->port.a |= JOY1_LEFT_BUTTON;
        }
        else if (pin & JOY1_LEFT_BUTTON)
        {
            sms->port.a |= JOY1_RIGHT_BUTTON;
        }
        if (pin & JOY1_UP_BUTTON)
        {
            sms->port.a |= JOY1_DOWN_BUTTON;
        }
        else if (pin & JOY1_DOWN_BUTTON)
        {
            sms->port.a |= JOY1_UP_BUTTON;
        }
    }
    else
    {
        sms->port.a |= pin;
    }
}

static void joypad_set_port_b(struct SMS_Core* sms, const uint8_t pin, const bool down)
{
    if (SMS_is_system_type_gg(sms))
    {
        if ((pin & PAUSE_BUTTON))
        {
            if (down)
            {
                sms->port.gg_regs[0x0] &= ~0x80;
            }
            else
            {
                sms->port.gg_regs[0x0] |= 0x80;
            }
        }
    }
    else
    {
        if (pin & PAUSE_BUTTON)
        {
            if (!is_button_down(sms->port.b, PAUSE_BUTTON) && down)
            {
                sms->vdp.nmi_pending = true;
            }
            else if (is_button_down(sms->port.b, PAUSE_BUTTON) && !down)
            {
                sms->vdp.nmi_pending = false;
            }
        }

        if ((pin & RESET_BUTTON) && (sms->port.b & RESET_BUTTON) && down)
        {
            // todo: handle reset pin
        }
    }

    if (down)
    {
        sms->port.b &= ~pin;
    }
    else
    {
        sms->port.b |= pin;
    }
}

void joypad_poll(struct SMS_Core* sms, int port)
{
    if (sms->input_callback)
    {
        sms->input_callback(sms->userdata, port);
    }
}

uint8_t joypad_read(struct SMS_Core* sms, int port)
{
    joypad_poll(sms, port);

    if (port == 0)
    {
        return sms->port.a;
    }
    else
    {
        uint8_t value = sms->port.b;

        if (SMS_is_system_type_gg(sms))
        {
            // GG (and MD) have no reset button so always return 1
            value |= RESET_BUTTON;
        }

        // pause button is on the console, and thus always reads 1 (i think)
        value |= PAUSE_BUTTON;

        return value;
    }
}

// [API]
void SMS_set_buttons(struct SMS_Core* sms, const uint16_t buttons, const bool down)
{
    joypad_set_port_a(sms, buttons >> 0, down);
    joypad_set_port_b(sms, buttons >> 8, down);
}
