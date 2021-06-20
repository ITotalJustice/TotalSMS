// this code is all based on the fantastic docs linked below
// SOURCE: https://www.smspower.org/uploads/Development/richard.txt
// SOURCE: https://www.smspower.org/uploads/Development/SN76489-20030421.txt

/* TODO:
    check what the noise counter reload should be (is it * 16?)
*/

#include "internal.h"


#define APU sms->apu


enum
{
    LATCH_TYPE_TONE = 0,
    LATCH_TYPE_NOISE = 0,
    LATCH_TYPE_VOL = 1,

    WHITE_NOISE = 1,
    PERIDOIC_NOISE = 0,

    // this depends on the system.
    // this should be a var instead, but ill add that when needed!
    TAPPED_BITS = 0x0009,

    LFSR_RESET_VALUE = 0x8000,
};


static inline void latch_reg_write(struct SMS_Core* sms, uint8_t value)
{
    APU.latched_channel = (value >> 5) & 0x3;
    APU.latched_type = (value >> 4) & 0x1;

    const uint8_t data = value & 0xF;

    switch (APU.latched_channel)
    {
        case 0: case 1: case 2:
            if (APU.latched_type == LATCH_TYPE_TONE)
            {
                APU.tone[APU.latched_channel].tone &= 0x3F0;
                APU.tone[APU.latched_channel].tone |= data;
            }
            else
            {
                APU.tone[APU.latched_channel].volume = data;
            }
            break;

        case 3:
            if (APU.latched_type == LATCH_TYPE_NOISE)
            {
                APU.noise.lfsr = LFSR_RESET_VALUE;
                APU.noise.shift_rate = data & 0x3;
                APU.noise.mode = (data >> 2) & 0x1;
            }
            else
            {
                APU.noise.volume = data;
            }
            break;
    }
}

static inline void data_reg_write(struct SMS_Core* sms, uint8_t value)
{
    const uint8_t data = value & 0x3F;

    switch (APU.latched_channel)
    {
        case 0: case 1: case 2:
            if (APU.latched_type == LATCH_TYPE_TONE)
            {
                APU.tone[APU.latched_channel].tone &= 0xF;
                APU.tone[APU.latched_channel].tone |= data << 4;
            }
            else
            {
                APU.tone[APU.latched_channel].volume = data & 0xF;
            }
            break;

        case 3:
            if (APU.latched_type == LATCH_TYPE_NOISE)
            {
                APU.noise.lfsr = LFSR_RESET_VALUE;
            }
            else
            {
                APU.noise.volume = data & 0xF;
            }
            break;
    }
}

void SN76489_reg_write(struct SMS_Core* sms, uint8_t value)
{
    // if MSB is set, then this is a latched write,
    // else, its a normal data write
    if (value & 0x80)
    {
        latch_reg_write(sms, value);
    }
    else
    {
        data_reg_write(sms, value);
    }
}

static inline void SN76489_tick_tone(struct SMS_Core* sms, uint8_t index, uint8_t cycles)
{
    // we don't want to keep change the polarity if the counter is already zero,
    // especially if the volume is still not off!
    // otherwise this will cause a hi-pitch screech, can be heard in golden-axe
    // to fix this, i check if the counter > 0 || if we have a value to reload
    // the counter with.
    if (APU.tone[index].counter || APU.tone[index].tone)
    {
        APU.tone[index].counter -= cycles;

        if (APU.tone[index].counter <= 0)
        {
            APU.tone[index].counter = APU.tone[index].tone * 16;
            // change the polarity
            APU.tone[index].polarity *= -1;
        }
    }
}

static inline void SN76489_tick_noise(struct SMS_Core* sms, uint8_t cycles)
{
    APU.noise.counter -= cycles;

    if (APU.noise.counter <= 0)
    {
        switch (APU.noise.shift_rate)
        {
            // i found that (freq * 8) or more sounds best for golden axe
            // when the lightning move is used, though this is probably a bug
            // with how my noise is implemented!
            case 0x0: APU.noise.counter = 0x10; break;
            case 0x1: APU.noise.counter = 0x20; break;
            case 0x2: APU.noise.counter = 0x40; break;
            case 0x3: APU.noise.counter = APU.tone[2].tone; break;
        }

        // change the polarity
        APU.noise.flip_flop = !APU.noise.flip_flop;

        // the nose is clocked every 2 countdowns!
        if (APU.noise.flip_flop == true)
        {
            // this is the bit used for the mixer
            APU.noise.shifted_bit = APU.noise.lfsr & 0x1 ? 1 : -1;

            if (APU.noise.mode == WHITE_NOISE)
            {
                APU.noise.lfsr = (APU.noise.lfsr >> 1) | (SMS_parity(APU.noise.lfsr & TAPPED_BITS) << 15);
            }
            else
            {
                APU.noise.lfsr = (APU.noise.lfsr >> 1) | ((APU.noise.lfsr & 0x1) << 15);
            }
        }
    }
}

// the volume is inverted, so that 0xF = OFF, 0x0 = MAX
static const int8_t VOLUME_INVERT_TABLE[] =
{
    0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
};

static inline int8_t SN76489_sample_tone(struct SMS_Core* sms, uint8_t index)
{
    return APU.tone[index].polarity * VOLUME_INVERT_TABLE[APU.tone[index].volume];
}

static inline int8_t SN76489_sample_noise(struct SMS_Core* sms)
{
    return APU.noise.shifted_bit * VOLUME_INVERT_TABLE[APU.noise.volume];
}

static void SN76489_sample(struct SMS_Core* sms)
{
    struct SMS_ApuCallbackData data =
    {
        .tone0 = SN76489_sample_tone(sms, 0),
        .tone1 = SN76489_sample_tone(sms, 1),
        .tone2 = SN76489_sample_tone(sms, 2),
        .noise = SN76489_sample_noise(sms),
    };

    APU.callback(APU.callback_user, &data);
}

void SN76489_run(struct SMS_Core* sms, uint8_t cycles)
{
    SN76489_tick_tone(sms, 0, cycles);
    SN76489_tick_tone(sms, 1, cycles);
    SN76489_tick_tone(sms, 2, cycles);
    SN76489_tick_noise(sms, cycles);

    if (APU.callback)
    {
        APU.counter -= cycles;

        if (APU.counter <= 0)
        {
            APU.counter += (CPU_CLOCK / APU.freq);
            SN76489_sample(sms);
        }
    }
}

void SN76489_init(struct SMS_Core* sms)
{
    APU.tone[0].polarity = +1;
    APU.tone[1].polarity = +1;
    APU.tone[2].polarity = +1;

    APU.tone[0].volume = 0xF;
    APU.tone[1].volume = 0xF;
    APU.tone[2].volume = 0xF;

    APU.noise.lfsr = LFSR_RESET_VALUE;
    APU.noise.mode = 0;
    APU.noise.shift_rate = 0;
    APU.noise.shifted_bit = 0;
    APU.noise.flip_flop = false;
    APU.noise.volume = 0xF;

    APU.latched_channel = 0;
}
