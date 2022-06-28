// this code is all based on the fantastic docs linked below
// SOURCE: https://www.smspower.org/uploads/Development/richard.txt
// SOURCE: https://www.smspower.org/uploads/Development/psg-20030421.txt

#include "internal.h"
#include <stdint.h>
#include <string.h>

#define PSG sms->psg

#if SMS_DISBALE_AUDIO

void psg_reg_write(struct SMS_Core* sms, const uint8_t value) {}
void psg_sync(struct SMS_Core* sms) {}
void psg_run(struct SMS_Core* sms, const uint8_t cycles) {}

#else

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


static FORCE_INLINE void latch_reg_write(struct SMS_Core* sms, const uint8_t value)
{
    PSG.latched_channel = (value >> 5) & 0x3;
    PSG.latched_type = (value >> 4) & 0x1;

    const uint8_t data = value & 0xF;

    if (PSG.latched_type == LATCH_TYPE_VOL)
    {
        PSG.volume[PSG.latched_channel] = data & 0xF;
    }
    else
    {
        switch (PSG.latched_channel)
        {
            case 0: case 1: case 2:
                PSG.tone[PSG.latched_channel].tone &= 0x3F0;
                PSG.tone[PSG.latched_channel].tone |= data;
                break;

            case 3:
                PSG.noise.lfsr = LFSR_RESET_VALUE;
                PSG.noise.shift_rate = data & 0x3;
                PSG.noise.mode = (data >> 2) & 0x1;
                break;
        }
    }
}

static FORCE_INLINE void data_reg_write(struct SMS_Core* sms, const uint8_t value)
{
    const uint8_t data = value & 0x3F;

    if (PSG.latched_type == LATCH_TYPE_VOL)
    {
        PSG.volume[PSG.latched_channel] = data & 0xF;
    }
    else
    {
        switch (PSG.latched_channel)
        {
            case 0: case 1: case 2:
                PSG.tone[PSG.latched_channel].tone &= 0xF;
                PSG.tone[PSG.latched_channel].tone |= data << 4;
                break;

            case 3:
                PSG.noise.lfsr = LFSR_RESET_VALUE;
                PSG.noise.shift_rate = data & 0x3;
                PSG.noise.mode = (data >> 2) & 0x1;
                break;
        }
    }
}

void psg_reg_write(struct SMS_Core* sms, const uint8_t value)
{
    psg_sync(sms);

    // if MSB is set, then this is a latched write, else its a normal data write
    if (value & 0x80)
    {
        latch_reg_write(sms, value);
    }
    else
    {
        data_reg_write(sms, value);
    }
}

static FORCE_INLINE void tick_tone(struct SMS_Core* sms, const uint8_t index, const uint8_t cycles)
{
    enum { MAX_SAMPLE_RATE = 8 }; // fixes shinobi tone2 (set to less than 8 to hear bug)

    // we don't want to keep change the polarity if the counter is already zero,
    // especially if the volume is still not off!
    // otherwise this will cause a hi-pitch screech, can be heard in golden-axe
    // to fix this, i check if the counter > 0 || if we have a value to reload
    // the counter with that's >= MAX_SAMPLE_RATE.
    if (PSG.tone[index].counter > 0 || PSG.tone[index].tone >= MAX_SAMPLE_RATE)
    {
        if (PSG.tone[index].counter > 0)
        {
            PSG.tone[index].counter -= cycles;
        }

        if (PSG.tone[index].counter <= 0)
        {
            // the apu runs x16 slower than cpu!
            PSG.tone[index].counter += PSG.tone[index].tone * 16;

            /*
                from the docs:

                Sample playback makes use of a feature of the psg's tone generators:
                when the half-wavelength (tone value) is set to 1, they output a DC offset
                value corresponding to the volume level (i.e. the wave does not flip-flop).
                By rapidly manipulating the volume, a crude form of PCM is obtained.
            */
            // this effect is used by the sega intro in Tail's Adventure and
            // sonic tripple trouble.
            if (PSG.tone[index].tone != 1)
            {
                // change the polarity
                PSG.polarity[index] ^= 1;
            }
        }
    }
}

static FORCE_INLINE void tick_noise(struct SMS_Core* sms, const uint8_t cycles)
{
    PSG.noise.counter -= cycles;

    if (PSG.noise.counter <= 0)
    {
        // the drums sound much better in basically every game if
        // the timer is 16, 32, 64.
        // however, the correct sound is at 256, 512, 1024.
        const uint8_t multi = sms->better_drums ? 1 : 16;

        // the apu runs x16 slower than cpu!
        switch (PSG.noise.shift_rate)
        {
            case 0x0: PSG.noise.counter += 1 * 16 * multi; break;
            case 0x1: PSG.noise.counter += 2 * 16 * multi; break;
            case 0x2: PSG.noise.counter += 4 * 16 * multi; break;
            // note: if tone == 0, this will cause a lot of random noise!
            case 0x3: PSG.noise.counter += PSG.tone[2].tone * 16; break;
        }

        // change the polarity
        PSG.noise.flip_flop ^= 1;

        // the nose is clocked every 2 countdowns!
        if (PSG.noise.flip_flop == true)
        {
            // this is the bit used for the mixer
            PSG.polarity[3] = (PSG.noise.lfsr & 0x1);

            if (PSG.noise.mode == WHITE_NOISE)
            {
                PSG.noise.lfsr = (PSG.noise.lfsr >> 1) | (SMS_parity16(PSG.noise.lfsr & TAPPED_BITS) << 15);
            }
            else
            {
                PSG.noise.lfsr = (PSG.noise.lfsr >> 1) | ((PSG.noise.lfsr & 0x1) << 15);
            }
        }
    }
}

static FORCE_INLINE uint8_t sample_channel(struct SMS_Core* sms, const uint8_t index)
{
    // the volume is inverted, so that 0xF = OFF, 0x0 = MAX
    return PSG.polarity[index] * (0xF - PSG.volume[index]);
}

static void sample(struct SMS_Core* sms)
{
    const uint8_t tone0 = sample_channel(sms, 0);
    const uint8_t tone1 = sample_channel(sms, 1);
    const uint8_t tone2 = sample_channel(sms, 2);
    // the noise channel sounds louder on actual console
    const uint8_t noise = sample_channel(sms, 3);

    struct SMS_ApuCallbackData data =
    {
        .tone0 = { tone0 * PSG.channel_enable[0][0], tone0 * PSG.channel_enable[0][1] },
        .tone1 = { tone1 * PSG.channel_enable[1][0], tone1 * PSG.channel_enable[1][1] },
        .tone2 = { tone2 * PSG.channel_enable[2][0], tone2 * PSG.channel_enable[2][1] },
        .noise = { noise * PSG.channel_enable[3][0], noise * PSG.channel_enable[3][1] },
    };

    sms->apu_callback(sms->userdata, &data);
}

// this is called on psg_reg_write() and at the end of a frame
void psg_sync(struct SMS_Core* sms)
{
    // psg regs cannot be read, so no point ticking stuff
    // if we don't have callback for samples to be pushed
    if (!sms->apu_callback)
    {
        return;
    }

    // psg is 16x slower than the cpu, so, it only makes sense to tick
    // each component at every 16 step.
    enum { STEP = 16 };

    // this loop will *not* cause PSG.cycles to underflow!
    for (; STEP <= PSG.cycles; PSG.cycles -= STEP)
    {
        tick_tone(sms, 0, STEP);
        tick_tone(sms, 1, STEP);
        tick_tone(sms, 2, STEP);
        tick_noise(sms, STEP);

        sms->apu_callback_counter += STEP;

        while (sms->apu_callback_counter >= sms->apu_callback_freq)
        {
            sms->apu_callback_counter -= sms->apu_callback_freq;
            sample(sms);
        }
    }
}

void psg_run(struct SMS_Core* sms, const uint8_t cycles)
{
    PSG.cycles += cycles; // PSG.cycles is an uint32_t, so it won't overflow
}

#endif // SMS_DISBALE_AUDIO

void psg_init(struct SMS_Core* sms)
{
    // by default, all channels are enabled in GG mode.
    // as sms is mono, these values will not be changed elsewhere
    // (so always enabled!).
    memset(PSG.channel_enable, true, sizeof(PSG.channel_enable));
    memset(PSG.polarity, 1, sizeof(PSG.polarity));
    memset(PSG.volume, 0xF, sizeof(PSG.volume));

    PSG.noise.lfsr = LFSR_RESET_VALUE;
    PSG.noise.mode = 0;
    PSG.noise.shift_rate = 0;
    PSG.noise.flip_flop = true;

    PSG.latched_channel = 0;
}
