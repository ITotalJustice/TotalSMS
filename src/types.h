#pragma once


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


enum Z80_Flags {
	Z80_FLAG_C  = 1 << 0, // set if result is > 255
	Z80_FLAG_N	= 1 << 1, // idk, ued for daa
	Z80_FLAG_P	= 1 << 2, // parity
	Z80_FLAG_V	= 1 << 2, // overflow
	Z80_FLAG_C3	= 1 << 3, // set to bit-3 of a result
	Z80_FLAG_H	= 1 << 4, // set if carry from bit-3 and bit-4
	Z80_FLAG_C5 = 1 << 5, // set to bit-5 of a result
	Z80_FLAG_Z	= 1 << 6, // set if the result is 0
	Z80_FLAG_S	= 1 << 7, // set to bit-7 of a result
};

enum Z80_RegisterSet {
	REGISTER_SET_MAIN = 0,
	REGISTER_SET_ALT = 1
};

enum Z80_8bitGeneralRegisters {
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

enum Z80_16bitGeneralRegisters {
	GENERAL_REGISTER_AF,
	GENERAL_REGISTER_BC,
	GENERAL_REGISTER_DE,
	GENERAL_REGISTER_HL,
};

enum Z80_8bitSpecialRegisters {
	SPECIAL_REGISTER_IXL,
	SPECIAL_REGISTER_IXH,
	SPECIAL_REGISTER_IYL,
	SPECIAL_REGISTER_IYH,

	SPECIAL_REGISTER_I,
	SPECIAL_REGISTER_R,
};

enum Z80_16bitSpecialRegisters {
	SPECIAL_REGISTER_PC, // program counter
	SPECIAL_REGISTER_SP, // stak pointer

	SPECIAL_REGISTER_IX,
	SPECIAL_REGISTER_IY,
};

struct Z80_GeneralRegisterSet
{
	uint8_t A;
	uint8_t F;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
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
	struct Z80_GeneralRegisterSet gregs[2];
};

enum
{
	SMS_ROM_SIZE_MAX = 0x80000 // 512KiB
};

struct SMS_Cart
{
	uint8_t rom[SMS_ROM_SIZE_MAX];
	uint32_t rom_size;
	uint32_t rom_mask;
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
};