#include "internal.h"

#include <stdio.h>
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

uint16_t SMS_read16(struct SMS_Core* sms, uint16_t addr)
{
	const uint8_t lo = SMS_read8(sms, addr + 0);
	const uint8_t hi = SMS_read8(sms, addr + 1);

	return (hi << 8) | lo;
}

void SMS_write16(struct SMS_Core* sms, uint16_t addr, uint16_t value)
{
	SMS_write8(sms, addr + 0, value & 0xFF);
    SMS_write8(sms, addr + 1, value >> 8);
}


// [IO]
uint8_t SMS_read_io(struct SMS_Core* sms, uint8_t addr)
{
	switch (addr)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B:
		case 0x2C: case 0x2D: case 0x2E: case 0x2F:
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B:
		case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			printf("[PORT-READ] 0x%02X last byte of the instruction\n", addr);
			break;

		case 0x40: case 0x42: case 0x44: case 0x46:
		case 0x48: case 0x4A: case 0x4C: case 0x4E:
		case 0x50: case 0x52: case 0x54: case 0x56:
		case 0x58: case 0x5A: case 0x5C: case 0x5E:
		case 0x60: case 0x62: case 0x64: case 0x66:
		case 0x68: case 0x6A: case 0x6C: case 0x6E:
		case 0x70: case 0x72: case 0x74: case 0x76:
		case 0x78: case 0x7A: case 0x7C: case 0x7E:
			printf("[PORT-READ] 0x%02X V counter\n", addr);
			break;

		case 0x41: case 0x43: case 0x45: case 0x47:
		case 0x49: case 0x4B: case 0x4D: case 0x4F:
		case 0x51: case 0x53: case 0x55: case 0x57:
		case 0x59: case 0x5B: case 0x5D: case 0x5F:
		case 0x61: case 0x63: case 0x65: case 0x67:
		case 0x69: case 0x6B: case 0x6D: case 0x6F:
		case 0x71: case 0x73: case 0x75: case 0x77:
		case 0x79: case 0x7B: case 0x7D: case 0x7F:
			printf("[PORT-READ] 0x%02X H counter\n", addr);
			break;

		case 0x80: case 0x82: case 0x84: case 0x86:
		case 0x88: case 0x8A: case 0x8C: case 0x8E:
		case 0x90: case 0x92: case 0x94: case 0x96:
		case 0x98: case 0x9A: case 0x9C: case 0x9E:
		case 0xA0: case 0xA2: case 0xA4: case 0xA6:
		case 0xA8: case 0xAA: case 0xAC: case 0xAE:
		case 0xB0: case 0xB2: case 0xB4: case 0xB6:
		case 0xB8: case 0xBA: case 0xBC: case 0xBE:
			printf("[PORT-READ] 0x%02X VDP data\n", addr);
			break;

		case 0x81: case 0x83: case 0x85: case 0x87:
		case 0x89: case 0x8B: case 0x8D: case 0x8F:
		case 0x91: case 0x93: case 0x95: case 0x97:
		case 0x99: case 0x9B: case 0x9D: case 0x9F:
		case 0xA1: case 0xA3: case 0xA5: case 0xA7:
		case 0xA9: case 0xAB: case 0xAD: case 0xAF:
		case 0xB1: case 0xB3: case 0xB5: case 0xB7:
		case 0xB9: case 0xBB: case 0xBD: case 0xBF:
			printf("[PORT-READ] 0x%02X VDP status flags\n", addr);
			break;

		case 0xC0: case 0xC2: case 0xC4: case 0xC6:
		case 0xC8: case 0xCA: case 0xCC: case 0xCE:
		case 0xD0: case 0xD2: case 0xD4: case 0xD6:
		case 0xD8: case 0xDA: case 0xDC: case 0xDE:
		case 0xE0: case 0xE2: case 0xE4: case 0xE6:
		case 0xE8: case 0xEA: case 0xEC: case 0xEE:
		case 0xF0: case 0xF2: case 0xF4: case 0xF6:
		case 0xF8: case 0xFA: case 0xFC: case 0xFE:
			printf("[PORT-READ] 0x%02X A\n", addr);
			break;

		case 0xC1: case 0xC3: case 0xC5: case 0xC7:
		case 0xC9: case 0xCB: case 0xCD: case 0xCF:
		case 0xD1: case 0xD3: case 0xD5: case 0xD7:
		case 0xD9: case 0xDB: case 0xDD: case 0xDF:
		case 0xE1: case 0xE3: case 0xE5: case 0xE7:
		case 0xE9: case 0xEB: case 0xED: case 0xEF:
		case 0xF1: case 0xF3: case 0xF5: case 0xF7:
		case 0xF9: case 0xFB: case 0xFD: case 0xFF:
			printf("[PORT-READ] 0x%02X B\n", addr);
			break;
	}

	return 0xFF;
}

void SMS_write_io(struct SMS_Core* sms, uint8_t addr, uint8_t value)
{
	switch (addr)
	{
		case 0x00: case 0x02: case 0x04: case 0x06:
		case 0x08: case 0x0A: case 0x0C: case 0x0E:
		case 0x10: case 0x12: case 0x14: case 0x16:
		case 0x18: case 0x1A: case 0x1C: case 0x1E:
		case 0x20: case 0x22: case 0x24: case 0x26:
		case 0x28: case 0x2A: case 0x2C: case 0x2E:
		case 0x30: case 0x32: case 0x34: case 0x36:
		case 0x38: case 0x3A: case 0x3C: case 0x3E:
			printf("[PORT-WRITE] 0x%02X Memory Control\n", addr);
			break;

		case 0x01: case 0x03: case 0x05: case 0x07:
		case 0x09: case 0x0B: case 0x0D: case 0x0F:
		case 0x11: case 0x13: case 0x15: case 0x17:
		case 0x19: case 0x1B: case 0x1D: case 0x1F:
		case 0x21: case 0x23: case 0x25: case 0x27:
		case 0x29: case 0x2B: case 0x2D: case 0x2F:
		case 0x31: case 0x33: case 0x35: case 0x37:
		case 0x39: case 0x3B: case 0x3D: case 0x3F:
			printf("[PORT-WRITE] 0x%02X IO Control\n", addr);
			break;

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B:
		case 0x4C: case 0x4D: case 0x4E: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B:
		case 0x6C: case 0x6D: case 0x6E: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			printf("[PORT-WRITE] 0x%02X SN76489 PSG\n", addr);
			break;

		case 0x80: case 0x82: case 0x84: case 0x86:
		case 0x88: case 0x8A: case 0x8C: case 0x8E:
		case 0x90: case 0x92: case 0x94: case 0x96:
		case 0x98: case 0x9A: case 0x9C: case 0x9E:
		case 0xA0: case 0xA2: case 0xA4: case 0xA6:
		case 0xA8: case 0xAA: case 0xAC: case 0xAE:
		case 0xB0: case 0xB2: case 0xB4: case 0xB6:
		case 0xB8: case 0xBA: case 0xBC: case 0xBE:
			printf("[PORT-WRITE] 0x%02X VDP data\n", addr);
			break;

		case 0x81: case 0x83: case 0x85: case 0x87:
		case 0x89: case 0x8B: case 0x8D: case 0x8F:
		case 0x91: case 0x93: case 0x95: case 0x97:
		case 0x99: case 0x9B: case 0x9D: case 0x9F:
		case 0xA1: case 0xA3: case 0xA5: case 0xA7:
		case 0xA9: case 0xAB: case 0xAD: case 0xAF:
		case 0xB1: case 0xB3: case 0xB5: case 0xB7:
		case 0xB9: case 0xBB: case 0xBD: case 0xBF:
			printf("[PORT-WRITE] 0x%02X VDP control flags\n", addr);
			break;
	}
}
