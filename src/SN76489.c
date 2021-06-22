// this code is all based on the fantastic docs linked below
// SOURCE: https://www.smspower.org/uploads/Development/richard.txt
// SOURCE: https://www.smspower.org/uploads/Development/SN76489-20030421.txt

#include "internal.h"
#include <string.h>


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

    if (APU.latched_type == LATCH_TYPE_VOL)
    {
        APU.volume[APU.latched_channel] = data & 0xF;
    }
    else
    {
        switch (APU.latched_channel)
        {
            case 0: case 1: case 2:
                APU.tone[APU.latched_channel].tone &= 0x3F0;
                APU.tone[APU.latched_channel].tone |= data;
                break;

            case 3:
                APU.noise.lfsr = LFSR_RESET_VALUE;
                APU.noise.shift_rate = data & 0x3;
                APU.noise.mode = (data >> 2) & 0x1;
                break;
        }
    }
}

static inline void data_reg_write(struct SMS_Core* sms, uint8_t value)
{
    const uint8_t data = value & 0x3F;

    if (APU.latched_type == LATCH_TYPE_VOL)
    {
        APU.volume[APU.latched_channel] = data & 0xF;
    }
    else
    {
        switch (APU.latched_channel)
        {
            case 0: case 1: case 2:
                APU.tone[APU.latched_channel].tone &= 0xF;
                APU.tone[APU.latched_channel].tone |= data << 4;
                break;

            case 3:
                APU.noise.lfsr = LFSR_RESET_VALUE;
                APU.noise.shift_rate = data & 0x3;
                APU.noise.mode = (data >> 2) & 0x1;
                break;
        }
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
            // the apu runs x16 slower than cpu!
            APU.tone[index].counter = APU.tone[index].tone * 16;
            // change the polarity
            APU.polarity[index] *= -1;
        }
    }
}

static inline void SN76489_tick_noise(struct SMS_Core* sms, uint8_t cycles)
{
    APU.noise.counter -= cycles;

    if (APU.noise.counter <= 0)
    {
        // the drums sound much better in basically every game if
        // the timer is 16, 32, 64.
        // however, the correct sound is at 256, 512, 1024.
        const uint8_t multi = APU.noise.better_drums ? 1 : 16;

        // the apu runs x16 slower than cpu!
        switch (APU.noise.shift_rate)
        {
            case 0x0: APU.noise.counter = 1 * 16 * multi; break;
            case 0x1: APU.noise.counter = 2 * 16 * multi; break;
            case 0x2: APU.noise.counter = 4 * 16 * multi; break;
            case 0x3: APU.noise.counter = APU.tone[2].tone * 16; break;
        }

        // change the polarity
        APU.noise.flip_flop = !APU.noise.flip_flop;

        // the nose is clocked every 2 countdowns!
        if (APU.noise.flip_flop == true)
        {
            // this is the bit used for the mixer
            APU.polarity[3] = APU.noise.lfsr & 0x1 ? +1 : -1;

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
static const int8_t VOLUME_INVERT_TABLE[0x10] =
{
    0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
};

static inline int8_t SN76489_sample_channel(struct SMS_Core* sms, uint8_t index)
{
    return APU.polarity[index] * VOLUME_INVERT_TABLE[APU.volume[index]];
}

static void SN76489_sample(struct SMS_Core* sms)
{
    struct SMS_ApuCallbackData data =
    {
        .tone0 = SN76489_sample_channel(sms, 0),
        .tone1 = SN76489_sample_channel(sms, 1),
        .tone2 = SN76489_sample_channel(sms, 2),
        .noise = SN76489_sample_channel(sms, 3),
    };

    sms->apu_callback(sms->apu_callback_user, &data);
}

void SN76489_run(struct SMS_Core* sms, uint8_t cycles)
{
    SN76489_tick_tone(sms, 0, cycles);
    SN76489_tick_tone(sms, 1, cycles);
    SN76489_tick_tone(sms, 2, cycles);
    SN76489_tick_noise(sms, cycles);

    if (sms->apu_callback)
    {
        sms->apu_callback_counter -= cycles;

        if (sms->apu_callback_counter <= 0)
        {
            sms->apu_callback_counter += (CPU_CLOCK / sms->apu_callback_freq);
            SN76489_sample(sms);
        }
    }
}

void SN76489_init(struct SMS_Core* sms)
{
    memset(APU.polarity, 1, sizeof(APU.polarity));
    memset(APU.volume, 0xF, sizeof(APU.volume));

    APU.noise.lfsr = LFSR_RESET_VALUE;
    APU.noise.mode = 0;
    APU.noise.shift_rate = 0;
    APU.noise.flip_flop = false;
    APU.noise.better_drums = false;

    APU.latched_channel = 0;
}
