#include "internal.h"


#define VDP sms->vdp


uint8_t VDP_status_flag_read(const struct SMS_Core* sms)
{
	return (VDP.frame_interrupt_pending << 7) |
		(VDP.sprite_overflow << 6) |
		(VDP.sprite_collision << 5);
}

