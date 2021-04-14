#include "internal.h"


#define G_REGS z80->gregs

#define REG_A G_REGS[0].A
#define REG_F G_REGS[0].F
#define REG_B G_REGS[0].B
#define REG_C G_REGS[0].C
#define REG_D G_REGS[0].D
#define REG_E G_REGS[0].E
#define REG_H G_REGS[0].H
#define REG_L G_REGS[0].L

#define REG_A_ALT G_REGS[1].A
#define REG_F_ALT G_REGS[1].F
#define REG_B_ALT G_REGS[1].B
#define REG_C_ALT G_REGS[1].C
#define REG_D_ALT G_REGS[1].D
#define REG_E_ALT G_REGS[1].E
#define REG_H_ALT G_REGS[1].H
#define REG_L_ALT G_REGS[1].L

#define REG_PC z80->PC
#define REG_SP z80->SP
#define REG_IXL z80->IXL
#define REG_IXH z80->IXH
#define REG_IYL z80->IYL
#define REG_IYH z80->IYH
#define REG_I z80->I
#define REG_R z80->R

#define GET_FLAG_C() IS_BIT_SET(REG_F, Z80_FLAG_C)
#define GET_FLAG_N() IS_BIT_SET(REG_F, Z80_FLAG_N)
#define GET_FLAG_P() IS_BIT_SET(REG_F, Z80_FLAG_P)
#define GET_FLAG_V() IS_BIT_SET(REG_F, Z80_FLAG_V)
#define GET_FLAG_H() IS_BIT_SET(REG_F, Z80_FLAG_H)
#define GET_FLAG_Z() IS_BIT_SET(REG_F, Z80_FLAG_Z)
#define GET_FLAG_S() IS_BIT_SET(REG_F, Z80_FLAG_S)

#define SET_FLAG_C(v) SET_BIT(REG_F, Z80_FLAG_C, v)
#define SET_FLAG_N(v) SET_BIT(REG_F, Z80_FLAG_N, v)
#define SET_FLAG_P(v) SET_BIT(REG_F, Z80_FLAG_P, v)
#define SET_FLAG_V(v) SET_BIT(REG_F, Z80_FLAG_V, v)
#define SET_FLAG_H(v) SET_BIT(REG_F, Z80_FLAG_H, v)
#define SET_FLAG_Z(v) SET_BIT(REG_F, Z80_FLAG_Z, v)
#define SET_FLAG_S(v) SET_BIT(REG_F, Z80_FLAG_S, v)


static inline void set_8bit_pair(uint8_t* hi, uint8_t* lo, uint16_t value)
{
	*hi = value >> 8;
	*lo = value & 0xFF;
}

static inline uint16_t get_8bit_pair(uint8_t hi, uint8_t lo)
{
	return (hi << 8) | lo;
}

void Z80_set_8bit_general_register(struct Z80* z80, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx, uint8_t value)
{
	switch (idx)
	{
		case GENERAL_REGISTER_A: G_REGS[set].A = value; break;
		case GENERAL_REGISTER_F: G_REGS[set].F = value; break;
		case GENERAL_REGISTER_B: G_REGS[set].B = value; break;
		case GENERAL_REGISTER_C: G_REGS[set].C = value; break;
		case GENERAL_REGISTER_D: G_REGS[set].D = value; break;
		case GENERAL_REGISTER_E: G_REGS[set].E = value; break;
		case GENERAL_REGISTER_H: G_REGS[set].H = value; break;
		case GENERAL_REGISTER_L: G_REGS[set].L = value; break;
	}
}

uint8_t Z80_get_8bit_general_register(const struct Z80* z80, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx)
{
	switch (idx)
	{
		case GENERAL_REGISTER_A: return G_REGS[set].A;
		case GENERAL_REGISTER_F: return G_REGS[set].F;
		case GENERAL_REGISTER_B: return G_REGS[set].B;
		case GENERAL_REGISTER_C: return G_REGS[set].C;
		case GENERAL_REGISTER_D: return G_REGS[set].D;
		case GENERAL_REGISTER_E: return G_REGS[set].E;
		case GENERAL_REGISTER_H: return G_REGS[set].H;
		case GENERAL_REGISTER_L: return G_REGS[set].L;
	}

	SMS_UNREACHABLE(0xFF);
}

void Z80_set_16bit_general_register(struct Z80* z80, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx, uint16_t value)
{
	switch (idx)
	{
		case GENERAL_REGISTER_AF: set_8bit_pair(&G_REGS[set].A, &G_REGS[set].F, value); break;
		case GENERAL_REGISTER_BC: set_8bit_pair(&G_REGS[set].B, &G_REGS[set].C, value); break;
		case GENERAL_REGISTER_DE: set_8bit_pair(&G_REGS[set].D, &G_REGS[set].E, value); break;
		case GENERAL_REGISTER_HL: set_8bit_pair(&G_REGS[set].H, &G_REGS[set].L, value); break;
	}
}

uint16_t Z80_get_16bit_general_register(const struct Z80* z80, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx)
{
	switch (idx)
	{
		case GENERAL_REGISTER_AF: return get_8bit_pair(G_REGS[set].A, G_REGS[set].F);
		case GENERAL_REGISTER_BC: return get_8bit_pair(G_REGS[set].B, G_REGS[set].C);
		case GENERAL_REGISTER_DE: return get_8bit_pair(G_REGS[set].D, G_REGS[set].E);
		case GENERAL_REGISTER_HL: return get_8bit_pair(G_REGS[set].H, G_REGS[set].L);
	}

	SMS_UNREACHABLE(0xFF);
}

void Z80_set_8bit_special_register(struct Z80* z80, enum Z80_8bitSpecialRegisters idx, uint8_t value)
{
	switch (idx)
	{
		case SPECIAL_REGISTER_IXL: REG_IXL = value; break;
		case SPECIAL_REGISTER_IXH: REG_IXH = value; break;
		case SPECIAL_REGISTER_IYL: REG_IYL = value; break;
		case SPECIAL_REGISTER_IYH: REG_IYH = value; break;
		case SPECIAL_REGISTER_I: REG_I = value; break;
		case SPECIAL_REGISTER_R: REG_R = value; break;
	}
}

uint8_t Z80_get_8bit_special_register(const struct Z80* z80, enum Z80_8bitSpecialRegisters idx)
{
	switch (idx)
	{
		case SPECIAL_REGISTER_IXL: return REG_IXL;
		case SPECIAL_REGISTER_IXH: return REG_IXH;
		case SPECIAL_REGISTER_IYL: return REG_IYL;
		case SPECIAL_REGISTER_IYH: return REG_IYH;
		case SPECIAL_REGISTER_I: return REG_I;
		case SPECIAL_REGISTER_R: return REG_R;
	}

	SMS_UNREACHABLE(0xFF);
}

void Z80_set_16bit_special_register(struct Z80* z80, enum Z80_16bitSpecialRegisters idx, uint16_t value)
{
	switch (idx)
	{
		case SPECIAL_REGISTER_PC: REG_PC = value; break;
		case SPECIAL_REGISTER_SP: REG_SP = value; break;
		case SPECIAL_REGISTER_IX: set_8bit_pair(&REG_IXH, &REG_IXL, value); break;
		case SPECIAL_REGISTER_IY: set_8bit_pair(&REG_IYH, &REG_IYL, value); break;
	}
}

uint16_t Z80_get_16bit_special_register(const struct Z80* z80, enum Z80_16bitSpecialRegisters idx)
{
	switch (idx)
	{
		case SPECIAL_REGISTER_PC: return REG_PC;
		case SPECIAL_REGISTER_SP: return REG_SP;
		case SPECIAL_REGISTER_IX: return get_8bit_pair(REG_IXH, REG_IXL);
		case SPECIAL_REGISTER_IY: return get_8bit_pair(REG_IYH, REG_IYL);
	}

	SMS_UNREACHABLE(0xFF);
}
