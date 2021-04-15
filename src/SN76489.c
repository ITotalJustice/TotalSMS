#include "internal.h"


#define APU sms->apu


enum
{
	LATCH_TYPE_TONE = 0,
	LATCH_TYPE_NOISE = 0,
	LATCH_TYPE_VOL = 1,
};


static void latch_reg_write(struct SMS_Core* sms, uint8_t value)
{
	APU.latched_channel = (value >> 5) & 0x3;
	APU.latched_type = (value >> 4) & 0x1;

	const uint8_t data = value & 0xF;

	switch (APU.latched_channel)
	{
		case 0: case 1: case 2:
			if (APU.latched_type == LATCH_TYPE_TONE)
			{
				APU.tone_channels[APU.latched_channel].tone |= data;
			}
			else
			{
				APU.tone_channels[APU.latched_channel].vol = data;
			}
			break;

		case 3:
			if (APU.latched_type == LATCH_TYPE_NOISE)
			{
				APU.noise_channel.shift_rate = data & 0x3;
				APU.noise_channel.mode = (data >> 2) & 1;
			}
			else
			{
				APU.tone_channels[APU.latched_channel].vol = data;
			}
			break;
	}
}

static void data_reg_write(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t data = value & 0x7F;

	switch (APU.latched_channel)
	{
		case 0: case 1: case 2:
			if (APU.latched_type == LATCH_TYPE_TONE)
			{
				APU.tone_channels[APU.latched_channel].tone &= 0xF;
				APU.tone_channels[APU.latched_channel].tone |= data << 4;
			}
			else
			{
				APU.tone_channels[APU.latched_channel].vol = data;
			}
			break;

		case 3:
			if (APU.latched_type == LATCH_TYPE_NOISE)
			{
				APU.noise_channel.shift_rate = data & 0x3;
				APU.noise_channel.mode = (data >> 2) & 1;
			}
			else
			{
				APU.tone_channels[APU.latched_channel].vol = data;
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
