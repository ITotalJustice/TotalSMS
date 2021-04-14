#include "internal.h"

#include <string.h>
#include <assert.h>

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

static inline void sega_mapper_update_slot0(struct SMS_Core* sms)
{
	const size_t offset = 0x4000 * sms->cart.mappers.sega.fffe;

	// this is fixed, never updated!
	sms->cart.mappers.sega.banks[0x00] = sms->cart.rom;

	for (size_t i = 1; i < 0x10; ++i)
	{
		sms->cart.mappers.sega.banks[i + 0x00] = sms->cart.rom + offset + (0x0400 * i);
	}
}

static inline void sega_mapper_update_slot1(struct SMS_Core* sms)
{
	const size_t offset = 0x4000 * sms->cart.mappers.sega.fffe;

	for (size_t i = 0; i < 0x10; ++i)
	{
		sms->cart.mappers.sega.banks[i + 0x10] = sms->cart.rom + offset + (0x0400 * i);
	}
}

static inline void sega_mapper_update_slot2(struct SMS_Core* sms)
{
	const size_t offset = 0x4000 * sms->cart.mappers.sega.ffff;

	for (size_t i = 0; i < 0x10; ++i)
	{
		sms->cart.mappers.sega.banks[i + 0x20] = sms->cart.rom + offset + (0x0400 * i);
	}
}

void sega_mapper_setup(struct SMS_Core* sms)
{
	// control is reset to zero
	memset(&sms->cart.mappers.sega.fffc, 0, sizeof(sms->cart.mappers.sega.fffc));
	sms->cart.mappers.sega.fffd = 0;
	sms->cart.mappers.sega.fffe = 1;
	sms->cart.mappers.sega.ffff = 2;

	sega_mapper_update_slot0(sms);
	sega_mapper_update_slot1(sms);
	sega_mapper_update_slot2(sms);
}

// TODO:
void codemaster_mapper_setup(struct SMS_Core* sms)
{
	assert(0 && "unfinished codemasters mapper!");
	SMS_UNUSED(sms);
}

static inline uint8_t none_mapper_read(struct SMS_Core* sms, uint16_t addr)
{
	return sms->cart.rom[addr & sms->cart.rom_mask];
}

static inline uint8_t sega_mapper_read(struct SMS_Core* sms, uint16_t addr)
{
	return sms->cart.mappers.sega.banks[addr >> 10][addr & 0x3FF];
}

static inline uint8_t cart_read(struct SMS_Core* sms, uint16_t addr)
{
	switch (sms->cart.mapper_type)
	{
		case MAPPER_TYPE_NONE: return none_mapper_read(sms, addr);
		case MAPPER_TYPE_SEGA: return sega_mapper_read(sms, addr);
	}

	SMS_UNREACHABLE(0xFF);
}

static inline void cart_write(struct SMS_Core* sms, uint16_t addr, uint8_t value)
{
	SMS_UNUSED(sms); SMS_UNUSED(addr); SMS_UNUSED(value);
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
	switch (addr)
	{
		case 0xFFFC: // Cartridge RAM mapper control
			// TODO:
			break;

		case 0xFFFD: // Mapper slot 0 control
			sms->cart.mappers.sega.fffd = value;
			sega_mapper_update_slot0(sms);
			break;

		case 0xFFFE: // Mapper slot 1 control
			sms->cart.mappers.sega.fffe = value;
			sega_mapper_update_slot1(sms);
			break;

		case 0xFFFF: // Mapper slot 2 control
			sms->cart.mappers.sega.ffff = value;
			sega_mapper_update_slot2(sms);
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
