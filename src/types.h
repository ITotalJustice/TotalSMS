#pragma once


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


enum Z80_RegisterSet
{
	REGISTER_SET_MAIN = 0,
	REGISTER_SET_ALT = 1
};

enum Z80_8bitGeneralRegisters
{
	// general registers
	GENERAL_REGISTER_A,
	GENERAL_REGISTER_F,
	GENERAL_REGISTER_B,
	GENERAL_REGISTER_C,
	GENERAL_REGISTER_D,
	GENERAL_REGISTER_E,
	GENERAL_REGISTER_H,
	GENERAL_REGISTER_L,
};

enum Z80_16bitGeneralRegisters
{
	GENERAL_REGISTER_AF,
	GENERAL_REGISTER_BC,
	GENERAL_REGISTER_DE,
	GENERAL_REGISTER_HL,
};

enum Z80_8bitSpecialRegisters
{
	SPECIAL_REGISTER_IXL,
	SPECIAL_REGISTER_IXH,
	SPECIAL_REGISTER_IYL,
	SPECIAL_REGISTER_IYH,

	SPECIAL_REGISTER_I,
	SPECIAL_REGISTER_R,
};

enum Z80_16bitSpecialRegisters
{
	SPECIAL_REGISTER_PC, // program counter
	SPECIAL_REGISTER_SP, // stak pointer

	SPECIAL_REGISTER_IX,
	SPECIAL_REGISTER_IY,
};

struct Z80_GeneralRegisterSet
{
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t A;

	struct
	{
		bool C;
		bool N;
		bool P;
		bool H;
		bool Z;
		bool S;
	} flags;
};


// this is a full z80 core
struct Z80
{
	// [special purpose registers]
	uint16_t PC; // program counter
	uint16_t SP; // stak pointer

	// these are actually mainly 16-bit registers, however, some instructions
	// split these into lo / hi bytes, similar to the general_reg_set.
	uint8_t IXL;
	uint8_t IXH;
	uint8_t IYL;
	uint8_t IYH;

	uint8_t I; // interrupt vector
	uint8_t R; // memory refresh

	// [general purpose registers]
	// theres 2-sets, main and alt
	struct Z80_GeneralRegisterSet main;
	struct Z80_GeneralRegisterSet alt;

	// interrupt flipflops
	bool IFF1;
	bool IFF2;
};

enum
{
	SMS_ROM_SIZE_MAX = 0x80000 // 512KiB
};

enum SMS_MapperType
{
	MAPPER_TYPE_NONE,
	MAPPER_TYPE_SEGA,
};

struct SMS_SegaMapper
{
	// mapped every 0x400
	uint8_t* banks[48];

	struct // control
	{
		bool rom_write_enable;
		bool ram_enable_c0000_ffff;
		bool ram_enable_80000_bffff;
		bool ram_bank_select;
		uint8_t bank_shift;
	} fffc;

	uint8_t fffd;
	uint8_t fffe;
	uint8_t ffff;
};

struct SMS_CodemastersMapper
{
	// mapped every 0x4000
	uint8_t* banks[3];
};

struct SMS_Cart
{
	uint8_t rom[SMS_ROM_SIZE_MAX];
	uint32_t rom_size;
	uint32_t rom_mask;

	union
	{
		struct SMS_SegaMapper sega;
		struct SMS_CodemastersMapper codemasters;
	} mappers;

	enum SMS_MapperType mapper_type;
};

struct SMS_RomHeader
{
	uint8_t magic[0x8];
	uint16_t checksum;
	uint32_t prod_code : 20;
	uint8_t version : 4;
	uint8_t region_code : 4;
	uint8_t rom_size : 4;
};

struct SMS_Core
{
	struct Z80 cpu;
	struct SMS_Cart cart;

	uint8_t system_ram[0x2000];
};
