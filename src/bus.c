#include "internal.h"


/*
$0000-$bfff	Cartridge (ROM/RAM/etc) (or other attached hardware)
$c000-$dfff	System RAM
$e000-$ffff	System RAM (mirror)
*/

/*
$0000-$03ff	ROM (unpaged)
$0400-$3fff	ROM mapper slot 0
$4000-$7fff	ROM mapper slot 1
$8000-$bfff	ROM/RAM mapper slot 2
$c000-$dfff	System RAM
$e000-$ffff	System RAM (mirror)
$fff8	3D glasses control
$fff9-$fffb	3D glasses control (mirrors)
$fffc	Cartridge RAM mapper control
$fffd	Mapper slot 0 control
$fffe	Mapper slot 1 control
$ffff	Mapper slot 2 control
*/

// the first 1k of rom is always fixed
// the rest of the 15k is changeable

static inline uint8_t cart_read(struct SMS_Core* sms, uint16_t addr)
{
	(void)sms; (void)addr;
	return 0;
}

static inline void cart_write(struct SMS_Core* sms, uint16_t addr, uint8_t value)
{
	(void)sms; (void)addr; (void)value;
}

uint8_t SMS_read8(struct SMS_Core* sms, uint16_t addr)
{
	switch(addr >> 13)
	{
		case 0: case 1: case 2:
		case 3: case 4: case 5:
			return cart_read(sms, addr);

		case 6: case 7:
			return sms->system_ram[addr & 0x1FFF];
	}

	SMS_UNREACHABLE(0xFF);
}

static inline void hi_ffxx_write(struct SMS_Core* sms, uint16_t addr, uint8_t value)
{
	(void)sms; (void)addr; (void)value;

	switch (addr)
	{
		case 0xFFFC: // Cartridge RAM mapper control
			break;

		case 0xFFFD: // Mapper slot 0 control
			break;

		case 0xFFFE: // Mapper slot 1 control
			break;

		case 0xFFFF: // Mapper slot 2 control
			break;
	}
}

void SMS_write8(struct SMS_Core* sms, uint16_t addr, uint8_t value)
{
	switch(addr >> 13)
	{
		case 0: case 1: case 2:
		case 3: case 4: case 5:
			cart_write(sms, addr, value);
			break;

		case 6:
			sms->system_ram[addr & 0x1FFF] = value;
			break;

		case 7:
			sms->system_ram[addr & 0x1FFF] = value;
			hi_ffxx_write(sms, addr, value);
			break;
	}
}
