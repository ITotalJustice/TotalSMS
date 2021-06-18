#include "internal.h"

#include <string.h>
#include <assert.h>


#if 1
    #define PARITY(v) !__builtin_parity(v)
#else
    // SOURCE: https://www.smspower.org/uploads/Development/SN76489-20030421.txt
    static inline int bit_parity(int val)
    {
        val ^= val >> 8;
        val ^= val >> 4;
        val ^= val >> 2;
        val ^= val >> 1;
        return val;
    };
    #define PARITY(v) bit_parity(v)
#endif

enum
{
	FLAG_C_MASK = 1 << 0, // set if result is > 255
	FLAG_N_MASK	= 1 << 1, // idk, ued for daa
	FLAG_P_MASK	= 1 << 2, // parity
	FLAG_V_MASK	= 1 << 2, // overflow
	FLAG_H_MASK	= 1 << 4, // set if carry from bit-3 and bit-4
	FLAG_Z_MASK	= 1 << 6, // set if the result is 0
	FLAG_S_MASK	= 1 << 7, // set to bit-7 of a result
};

#define REG_B sms->cpu.main.B
#define REG_C sms->cpu.main.C
#define REG_D sms->cpu.main.D
#define REG_E sms->cpu.main.E
#define REG_H sms->cpu.main.H
#define REG_L sms->cpu.main.L
#define REG_A sms->cpu.main.A

#define REG_B_ALT sms->cpu.alt.B
#define REG_C_ALT sms->cpu.alt.C
#define REG_D_ALT sms->cpu.alt.D
#define REG_E_ALT sms->cpu.alt.E
#define REG_H_ALT sms->cpu.alt.H
#define REG_L_ALT sms->cpu.alt.L
#define REG_A_ALT sms->cpu.alt.A

#define REG_PC sms->cpu.PC
#define REG_SP sms->cpu.SP

#define REG_IXL sms->cpu.IXL
#define REG_IXH sms->cpu.IXH
#define REG_IYL sms->cpu.IYL
#define REG_IYH sms->cpu.IYH

#define REG_I sms->cpu.I
#define REG_R sms->cpu.R

#define FLAG_C sms->cpu.main.flags.C
#define FLAG_N sms->cpu.main.flags.N
#define FLAG_P sms->cpu.main.flags.P // P/V
#define FLAG_V sms->cpu.main.flags.P // P/V
#define FLAG_H sms->cpu.main.flags.H
#define FLAG_Z sms->cpu.main.flags.Z
#define FLAG_S sms->cpu.main.flags.S

#define FLAG_C_ALT sms->cpu.alt.flags.C
#define FLAG_N_ALT sms->cpu.alt.flags.N
#define FLAG_P_ALT sms->cpu.alt.flags.P // P/V
#define FLAG_V_ALT sms->cpu.alt.flags.P // P/V
#define FLAG_H_ALT sms->cpu.alt.flags.H
#define FLAG_Z_ALT sms->cpu.alt.flags.Z
#define FLAG_S_ALT sms->cpu.alt.flags.S

// REG_F getter
#define REG_F_GET() \
	((FLAG_S << 7) | (FLAG_Z << 6) | (FLAG_H << 4) | \
	(FLAG_V << 2) | (FLAG_N << 1) | (FLAG_C << 0))

#define REG_F_GET_ALT() \
	((FLAG_S_ALT << 7) | (FLAG_Z_ALT << 6) | (FLAG_H_ALT << 4) | \
	(FLAG_V_ALT << 2) | (FLAG_N_ALT << 1) | (FLAG_C_ALT << 0))

// REG_F setter
#define REG_F_SET(v) \
	FLAG_S = (v) & FLAG_S_MASK; \
	FLAG_Z = (v) & FLAG_Z_MASK; \
	FLAG_H = (v) & FLAG_H_MASK; \
	FLAG_V = (v) & FLAG_V_MASK; \
	FLAG_N = (v) & FLAG_N_MASK; \
	FLAG_C = (v) & FLAG_C_MASK

// REG_F setter
#define REG_F_SET_ALT(v) \
	FLAG_S_ALT = (v) & FLAG_S_MASK; \
	FLAG_Z_ALT = (v) & FLAG_Z_MASK; \
	FLAG_H_ALT = (v) & FLAG_H_MASK; \
	FLAG_V_ALT = (v) & FLAG_V_MASK; \
	FLAG_N_ALT = (v) & FLAG_N_MASK; \
	FLAG_C_ALT = (v) & FLAG_C_MASK

#define PAIR(hi, lo) ((hi << 8) | (lo))
#define SET_PAIR(hi, lo, v) hi = ((v) >> 8) & 0xFF; lo = (v) & 0xFF

// getters
#define REG_BC PAIR(REG_B, REG_C)
#define REG_DE PAIR(REG_D, REG_E)
#define REG_HL PAIR(REG_H, REG_L)
#define REG_AF PAIR(REG_A, REG_F_GET())
#define REG_IX PAIR(REG_IXH, REG_IXL)
#define REG_IY PAIR(REG_IYH, REG_IYL)

// todo: remove
#define REG_BC_ALT PAIR(REG_B_ALT, REG_C_ALT)
#define REG_DE_ALT PAIR(REG_D_ALT, REG_E_ALT)
#define REG_HL_ALT PAIR(REG_H_ALT, REG_L_ALT)
#define REG_AF_ALT PAIR(REG_A_ALT, REG_F_GET_ALT())

// setters
#define SET_REG_BC(v) SET_PAIR(REG_B, REG_C, v)
#define SET_REG_DE(v) SET_PAIR(REG_D, REG_E, v)
#define SET_REG_HL(v) SET_PAIR(REG_H, REG_L, v)
#define SET_REG_AF(v) REG_A = (((v) >> 8) & 0xFF); REG_F_SET(v)
#define SET_REG_IX(v) SET_PAIR(REG_IXH, REG_IXL, v)
#define SET_REG_IY(v) SET_PAIR(REG_IYH, REG_IYL, v)

// TODO: remove
#define SET_REG_BC_ALT(v) SET_PAIR(REG_B_ALT, REG_C_ALT, v)
#define SET_REG_DE_ALT(v) SET_PAIR(REG_D_ALT, REG_E_ALT, v)
#define SET_REG_HL_ALT(v) SET_PAIR(REG_H_ALT, REG_L_ALT, v)
#define SET_REG_AF_ALT(v) REG_A_ALT = (((v) >> 8) & 0xFF); REG_F_SET_ALT(v)

#define SET_FLAGS_NHZ(n,h,z) \
	FLAG_N = n; \
	FLAG_H = h; \
	FLAG_Z = z;

#define SET_ALL_FLAGS(c,n,p,h,z,s) \
	FLAG_C = c; \
	FLAG_N = n; \
	FLAG_P = p; \
	FLAG_H = h; \
	FLAG_Z = z; \
	FLAG_S = s;

static bool COND(const struct SMS_Core* sms, uint8_t idx)
{
	switch (idx & 0x7)
	{
		case 0: return FLAG_C == true;
		case 1: return FLAG_C == false;
		case 2: return FLAG_Z == true;
		case 3: return FLAG_Z == false;
		case 4: return FLAG_P == true;
		case 5: return FLAG_P == false;
		case 6: return FLAG_S == true;
		case 7: return FLAG_S == false;
	}

	UNREACHABLE(false);
}

#define read8(addr) SMS_read8(sms, addr)
#define read16(addr) SMS_read16(sms, addr)
#define write8(addr,value) SMS_write8(sms, addr, value)
#define write16(addr,value) SMS_write16(sms, addr, value)

#define readIO(addr) SMS_read_io(sms, addr)
#define writeIO(addr,value) SMS_write_io(sms, addr, value)

static uint8_t get_r8(struct SMS_Core* sms, uint8_t idx)
{
	switch (idx & 0x7)
	{
		case 0x0: return REG_B;
		case 0x1: return REG_C;
		case 0x2: return REG_D;
		case 0x3: return REG_E;
		case 0x4: return REG_H;
		case 0x5: return REG_L;
		case 0x6: return read8(REG_HL);
		case 0x7: return REG_A;
	}

	UNREACHABLE(0xFF);
}

static void set_r8(struct SMS_Core* sms, uint8_t value, uint8_t idx)
{
	switch (idx & 0x7)
	{
		case 0x0: REG_B = value; break;
		case 0x1: REG_C = value; break;
		case 0x2: REG_D = value; break;
		case 0x3: REG_E = value; break;
		case 0x4: REG_H = value; break;
		case 0x5: REG_L = value; break;
		case 0x6: write8(REG_HL, value); break;
		case 0x7: REG_A = value; break;
	}
}

static uint16_t get_r16(struct SMS_Core* sms, uint8_t idx)
{
	switch (idx & 0x3)
	{
		case 0x0: return REG_BC;
		case 0x1: return REG_DE;
		case 0x2: return REG_HL;
		case 0x3: return REG_SP;
	}

	UNREACHABLE(0xFF);
}

static void set_r16(struct SMS_Core* sms, uint16_t value, uint8_t idx)
{
	switch (idx & 0x3)
	{
		case 0x0: SET_REG_BC(value); break;
		case 0x1: SET_REG_DE(value); break;
		case 0x2: SET_REG_HL(value); break;
		case 0x3: REG_SP = value; break;
	}
}

static void _ADD(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A + value;
	
	FLAG_C = (REG_A + value) > 0xFF;
	FLAG_N = false;
	FLAG_H = ((REG_A & 0xF) + (value & 0xF)) > 0xF;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;

	REG_A = result;
}

static void _SUB(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A - value;
	
	FLAG_C = value > REG_A;
	FLAG_N = true;
	FLAG_H = (REG_A & 0xF) < (value & 0xF);
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;

	REG_A = result;
}

static void _AND(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A & value;

	FLAG_C = false;
	FLAG_N = false;
	FLAG_P = PARITY(result);
	FLAG_H = true;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;

	REG_A = result;
}

static void _XOR(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A ^ value;

	FLAG_C = false;
	FLAG_N = false;
	FLAG_P = PARITY(result);
	FLAG_H = false;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;

	REG_A = result;
}

static void _OR(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A | value;

	FLAG_C = false;
	FLAG_N = false;
	FLAG_P = PARITY(result);
	FLAG_H = false;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;

	REG_A = result;
}

static void _CP(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = REG_A - value;
	
	FLAG_C = value > REG_A;
	FLAG_N = true;
	FLAG_H = (REG_A & 0xF) < (value & 0xF);
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;
}

static void _ADC(struct SMS_Core* sms, uint8_t value)
{
	_ADD(sms, value + FLAG_C);
}

static void _SBC(struct SMS_Core* sms, uint8_t value)
{
	_SUB(sms, value + FLAG_C);
}

static void AND_r(struct SMS_Core* sms, uint8_t opcode)
{
	_AND(sms, get_r8(sms, opcode));
}

static void XOR_r(struct SMS_Core* sms, uint8_t opcode)
{
	_XOR(sms, get_r8(sms, opcode));
}

static void OR_r(struct SMS_Core* sms, uint8_t opcode)
{
	_OR(sms, get_r8(sms, opcode));
}

static void CP_r(struct SMS_Core* sms, uint8_t opcode)
{
	_CP(sms, get_r8(sms, opcode));
}

static void ADD_r(struct SMS_Core* sms, uint8_t opcode)
{
	_ADD(sms, get_r8(sms, opcode));
}

static void SUB_r(struct SMS_Core* sms, uint8_t opcode)
{
	_SUB(sms, get_r8(sms, opcode));
}

static void ADC_r(struct SMS_Core* sms, uint8_t opcode)
{
	_ADC(sms, get_r8(sms, opcode));
}

static void SBC_r(struct SMS_Core* sms, uint8_t opcode)
{
	_SBC(sms, get_r8(sms, opcode));
}

static void AND_imm(struct SMS_Core* sms)
{
	_AND(sms, read8(REG_PC++));
}

static void XOR_imm(struct SMS_Core* sms)
{
	_XOR(sms, read8(REG_PC++));
}

static void OR_imm(struct SMS_Core* sms)
{
	_OR(sms, read8(REG_PC++));
}

static void CP_imm(struct SMS_Core* sms)
{
	_CP(sms, read8(REG_PC++));
}

static void ADD_imm(struct SMS_Core* sms)
{
	_ADD(sms, read8(REG_PC++));
}

static void SUB_imm(struct SMS_Core* sms)
{
	_SUB(sms, read8(REG_PC++));
}

static void ADC_imm(struct SMS_Core* sms)
{
	_ADC(sms, read8(REG_PC++));
}

static void SBC_imm(struct SMS_Core* sms)
{
	_SBC(sms, read8(REG_PC++));
}

static void NEG(struct SMS_Core* sms)
{
    _SUB(sms, REG_A * 2); // 1 - (1*2) = 0xFF
}

static void ADC_hl(struct SMS_Core* sms, uint16_t value)
{
    value += FLAG_C;
    
    const uint16_t hl = REG_HL;
    const uint8_t result = hl + value;

    FLAG_C = (value + hl) > 0xFFFF;
    FLAG_N = false;
    FLAG_H = ((REG_A & 0xFFF) + (value & 0xFFF)) > 0xFFF;
    FLAG_Z = result == 0;
    FLAG_S = result >> 15;

    SET_REG_HL(result);
}

static void SBC_hl(struct SMS_Core* sms, uint16_t value)
{
    value += FLAG_C;

    const uint16_t hl = REG_HL;
    const uint8_t result = hl - value;
    
    FLAG_C = value > hl;
    FLAG_N = true;
    FLAG_H = (REG_A & 0xFFF) < (value & 0xFFF);
    FLAG_Z = result == 0;
    FLAG_S = result >> 15;

    SET_REG_HL(result);
}

static void PUSH(struct SMS_Core* sms, uint16_t value)
{
	write8(--REG_SP, (value >> 8) & 0xFF);
	write8(--REG_SP, value & 0xFF);
}

static uint16_t POP(struct SMS_Core* sms)
{
	const uint16_t result = read16(REG_SP);
	REG_SP += 2;
	return result;
}

static void shift_left_flags(struct SMS_Core* sms, uint8_t result, uint8_t value)
{
	FLAG_C = value >> 7;
	FLAG_N = false;
	FLAG_P = PARITY(result);
	FLAG_H = false;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;
}

static void shift_right_flags(struct SMS_Core* sms, uint8_t result, uint8_t value)
{
	FLAG_C = value & 0x1;
	FLAG_N = false;
	FLAG_P = PARITY(result);
	FLAG_H = false;
	FLAG_Z = result == 0;
	FLAG_S = result >> 7;
}

static void ADD_HL(struct SMS_Core* sms, uint8_t opcode)
{
	const uint16_t value = get_r16(sms, opcode >> 4);
	const uint16_t HL = REG_HL;
	const uint16_t result = HL + value;

	FLAG_C = HL + value > 0xFFFF;
	FLAG_H = (HL & 0xFFF) + (value & 0xFFF) > 0xFFF;
	FLAG_N = false;
	
	SET_REG_HL(result);
}

static void INC_r8(struct SMS_Core* sms, uint8_t opcode)
{
	const uint8_t result = get_r8(sms, opcode >> 3) + 1;
	set_r8(sms, result, opcode >> 3);
	
	FLAG_N = false;
	FLAG_H = (result & 0xF) == 0;
	FLAG_Z = result == 0;
}

static void DEC_r8(struct SMS_Core* sms, uint8_t opcode)
{
	const uint8_t result = get_r8(sms, opcode >> 3) - 1;
	set_r8(sms, result, opcode >> 3);
	
	FLAG_N = true;
	FLAG_H = (result & 0xF) == 0xF;
	FLAG_Z = result == 0;
}

static void INC_r16(struct SMS_Core* sms, uint8_t opcode)
{
	const uint8_t idx = opcode >> 4;
	set_r16(sms, get_r16(sms, idx) + 1, idx);
}

static void DEC_r16(struct SMS_Core* sms, uint8_t opcode)
{
	const uint8_t idx = opcode >> 4;
	set_r16(sms, get_r16(sms, idx) - 1, idx);
}

// static void LD_a_bc(struct SMS_Core* sms)
// {
// 	// loads the value at (BC) not BC!
// 	REG_A = read8(REG_BC);
// }

// static void LD_a_de(struct SMS_Core* sms)
// {
// 	// loads the value at (DE) not DE!
// 	REG_A = read8(REG_DE);
// }

// static void LD_bc_a(struct SMS_Core* sms)
// {
// 	// sets the value at (BC) not BC!
// 	write8(REG_BC, REG_A);
// }

// static void LD_de_a(struct SMS_Core* sms)
// {
// 	// sets the value at (DE) not DE!
// 	write8(REG_DE, REG_A);
// }

static void LD_imm_a(struct SMS_Core* sms)
{
	const uint16_t addr = read16(REG_PC);
	REG_PC += 2;

	write8(addr, REG_A);
}

static void LD_a_imm(struct SMS_Core* sms)
{
	const uint16_t addr = read16(REG_PC);
	REG_PC += 2;

	REG_A = read8(addr);
}

static void LD_imm_hl(struct SMS_Core* sms)
{
    const uint16_t addr = read16(REG_PC);
    REG_PC += 2;

    write16(addr, REG_HL);
}

static void LD_hl_imm(struct SMS_Core* sms)
{
    const uint16_t addr = read16(REG_PC);
    REG_PC += 2;

    const uint16_t r = read16(addr);
    SET_REG_HL(r);
}

static void LD_16(struct SMS_Core* sms, uint8_t opcode)
{
	const uint16_t value = read16(REG_PC);
	REG_PC += 2;
	set_r16(sms, value, opcode >> 4);
}

static void LD_r_u8(struct SMS_Core* sms, uint8_t opcode)
{
	set_r8(sms, read8(REG_PC++), opcode >> 3);
}

static void LD_rr(struct SMS_Core* sms, uint8_t opcode)
{
	set_r8(sms, get_r8(sms, opcode), opcode >> 3);
}

static void LD_sp_hl(struct SMS_Core* sms)
{
    REG_SP = REG_HL;
}

static void LD_r16_a(struct SMS_Core* sms, uint16_t r16)
{
    write8(r16, REG_A);
}

static void LD_a_r16(struct SMS_Core* sms, uint16_t r16)
{
    REG_A = read8(r16);
}

static void RST(struct SMS_Core* sms, uint16_t value)
{
	PUSH(sms, REG_PC);
	REG_PC = value;
}

static void CALL(struct SMS_Core* sms)
{
	const uint16_t value = read16(REG_PC);
	PUSH(sms, REG_PC + 2);
	REG_PC = value;
}

static void CALL_cc(struct SMS_Core* sms, uint8_t opcode)
{
	if (COND(sms, opcode))
	{
		CALL(sms);
	}
	else
	{
		REG_PC += 2;
	}
}

static void RET(struct SMS_Core* sms)
{
	REG_PC = POP(sms);
}

static void RET_cc(struct SMS_Core* sms, uint8_t opcode)
{
	if (COND(sms, opcode))
	{
		RET(sms);
	}
}

static void JR(struct SMS_Core* sms)
{
	REG_PC += ((int8_t)read8(REG_PC)) + 1;
}

static void DJNZ(struct SMS_Core* sms)
{
	--REG_B;

	if (REG_B)
	{
		JR(sms);
	}
	else
	{
		REG_PC += 1;
	}
}

static void JR_cc(struct SMS_Core* sms, uint8_t opcode)
{
	if (COND(sms, opcode))
	{
		JR(sms);
	}
	else
	{
		REG_PC += 1;
	}
}

static void JP(struct SMS_Core* sms)
{
	REG_PC = read16(REG_PC);
}

static void JP_cc(struct SMS_Core* sms, uint8_t opcode)
{
	if (COND(sms, opcode))
	{
		JP(sms);
	}
	else
	{
		REG_PC += 2;
	}
}

static void EI(struct SMS_Core* sms)
{
	sms->cpu.IFF1 = true;
}

static void DI(struct SMS_Core* sms)
{
	sms->cpu.IFF1 = false;
}

static uint8_t _RL(struct SMS_Core* sms, uint8_t value, bool carry)
{
	const uint8_t result = (value << 1) | carry;

	shift_left_flags(sms, result, value);
    
    return result;
}

static uint8_t _RR(struct SMS_Core* sms, uint8_t value, bool carry)
{
	const uint8_t result = (value >> 1) | (carry << 7);

	shift_right_flags(sms, result, value);

    return result;
}

// the accumulator shifts do not affect the [p, z, s] flags
static void RLA(struct SMS_Core* sms)
{
    const bool p = FLAG_P, z = FLAG_Z, s = FLAG_S;

    REG_A = _RL(sms, REG_A, FLAG_C);

    FLAG_P = p; FLAG_Z = z; FLAG_S = s;
}

static void RRA(struct SMS_Core* sms)
{
    const bool p = FLAG_P, z = FLAG_Z, s = FLAG_S;

    REG_A = _RR(sms, REG_A, FLAG_C);

    FLAG_P = p; FLAG_Z = z; FLAG_S = s;
}

static void RLCA(struct SMS_Core* sms)
{
    const bool p = FLAG_P, z = FLAG_Z, s = FLAG_S;

    REG_A = _RL(sms, REG_A, REG_A >> 7);

    FLAG_P = p; FLAG_Z = z; FLAG_S = s;
}

static void RRCA(struct SMS_Core* sms)
{
    const bool p = FLAG_P, z = FLAG_Z, s = FLAG_S;

    REG_A = _RR(sms, REG_A, REG_A & 1);

    FLAG_P = p; FLAG_Z = z; FLAG_S = s;
}

static uint8_t RL(struct SMS_Core* sms, uint8_t value)
{	
    return _RL(sms, value, FLAG_C);
}

static uint8_t RLC(struct SMS_Core* sms, uint8_t value)
{    
    return _RL(sms, value, value >> 7);
}

static uint8_t RR(struct SMS_Core* sms, uint8_t value)
{	
    return _RR(sms, value, FLAG_C);
}

static uint8_t RRC(struct SMS_Core* sms, uint8_t value)
{
	return _RR(sms, value, value & 1);
}

static uint8_t SLA(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = value << 1;
	
	shift_left_flags(sms, result, value);

    return result;
}

static uint8_t SRA(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = (value >> 1) | (value & 0x80);
	
	shift_right_flags(sms, result, value);

    return result;
}

static uint8_t SLL(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = (value << 1) | 0x1;
	
	shift_left_flags(sms, result, value);

    return result;
}

static uint8_t SRL(struct SMS_Core* sms, uint8_t value)
{
	const uint8_t result = value >> 1;
	
	shift_right_flags(sms, result, value);

    return result;
}

static void BIT(struct SMS_Core* sms, uint8_t value, uint8_t bit)
{
	const bool result = IS_BIT_SET(value, bit);
	
	SET_FLAGS_NHZ(false, true, result == 0);
}

static uint8_t RES(struct SMS_Core* sms, uint8_t value, uint8_t bit)
{
    UNUSED(sms);

	const uint8_t result = value & ~bit;
	
	return result;
}

static uint8_t SET(struct SMS_Core* sms, uint8_t value, uint8_t bit)
{
    UNUSED(sms);

	const uint8_t result = value | bit;
	
	return result;
}

static void IMM_set(struct SMS_Core* sms, uint8_t mode)
{
	assert(mode == 1 && "invalid mode set for SMS");
    UNUSED(sms); UNUSED(mode);
}

static void IN_imm(struct SMS_Core* sms)
{
	const uint8_t port = read8(REG_PC++);
	REG_A = readIO(port);
}

static void OUT_imm(struct SMS_Core* sms)
{
	const uint8_t port = read8(REG_PC++);
	writeIO(port, REG_A);
}

static void EX_af_af(struct SMS_Core* sms)
{
	const uint16_t temp = REG_AF_ALT;
	SET_REG_AF_ALT(REG_AF);
	SET_REG_AF(temp);
}

static void EXX(struct SMS_Core* sms)
{
	// BC
	uint16_t temp = REG_BC_ALT;
	SET_REG_BC_ALT(REG_BC);
	SET_REG_BC(temp);
	// DE
	temp = REG_DE_ALT;
	SET_REG_DE_ALT(REG_DE);
	SET_REG_DE(temp);
	// HL
	temp = REG_HL_ALT;
	SET_REG_HL_ALT(REG_HL);
	SET_REG_HL(temp);
}

static void EX_de_hl(struct SMS_Core* sms)
{
	const uint16_t temp = REG_DE;
	SET_REG_DE(REG_HL);
	SET_REG_HL(temp);
}

static void EX_sp_hl(struct SMS_Core* sms)
{
	// this swaps the value at (SP), not SP!
	const uint16_t value = read16(REG_SP);
	write16(REG_SP, REG_HL);
	SET_REG_HL(value);
}

// this can be optimised slightly by using
// a loop instead
static void LDIR(struct SMS_Core* sms)
{
    uint16_t hl = REG_HL;
    uint16_t de = REG_DE;
    uint16_t bc = REG_BC;

    // [DE] = [HL]
    // DE++; HL++; BC--;
    write8(de, read8(hl));

    ++hl; ++de; --bc;

    // this checks after decrement
    // meaning that if BC = 0 before decrement, then it'll
    // run for UINT16_MAX
    if (bc)
    {
        REG_PC -= 2;
    }

    SET_REG_BC(bc);
    SET_REG_DE(de);
    SET_REG_HL(hl);
}

// NOTE: templates would be much nicer here
// returns true if the result needs to be written back (all except BIT)
static bool _CB(struct SMS_Core* sms, uint8_t opcode, uint8_t value, uint8_t* result)
{
	switch ((opcode >> 3) & 0x1F)
    {
        case 0x00: *result = RLC(sms, value); return true;
        case 0x01: *result = RRC(sms, value); return true;
        case 0x02: *result = RL(sms, value);  return true;
        case 0x03: *result = RR(sms, value);  return true;
        case 0x04: *result = SLA(sms, value); return true;
        case 0x05: *result = SRA(sms, value); return true;
        case 0x06: *result = SLL(sms, value); return true;
        case 0x07: *result = SRL(sms, value); return true;

        case 0x08: case 0x09: case 0x0A: case 0x0B:
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
            BIT(sms, value, 1 << (opcode >> 3));
            return false;

        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
            *result = RES(sms, value, 1 << (opcode >> 3));
            return true;

        case 0x18: case 0x19: case 0x1A: case 0x1B:
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
            *result = SET(sms, value, 1 << (opcode >> 3));
            return true;
    }

    UNREACHABLE(false);
}

static void execute_CB(struct SMS_Core* sms)
{
    const uint8_t opcode = SMS_read8(sms, REG_PC++);
    const uint8_t value = get_r8(sms, opcode);

    uint8_t result = 0;

    if (_CB(sms, opcode, value, &result))
    {
        set_r8(sms, result, opcode);
    }
}

static void execute_CB_IXIY(struct SMS_Core* sms, uint16_t ixy)
{
    const uint8_t opcode = SMS_read8(sms, REG_PC++);
    const uint16_t addr = ixy + (int8_t)read8(REG_PC++);
    const uint8_t value = read8(addr);

    uint8_t result = 0;

    if (_CB(sms, opcode, value, &result))
    {
        switch (opcode & 0x7)
        {
            case 0x0: REG_B = result; break;
            case 0x1: REG_C = result; break;
            case 0x2: REG_D = result; break;
            case 0x3: REG_E = result; break;
            case 0x4: REG_H = result; break;
            case 0x5: REG_L = result; break;
            case 0x6: write8(addr, result); break;
            case 0x7: REG_A = result; break;
        }
    }
}

static void execute_IXIY(struct SMS_Core* sms, uint8_t* ixy_hi, uint8_t* ixy_lo)
{
    const uint8_t opcode = SMS_read8(sms, REG_PC++);
    const uint16_t pair = PAIR(*ixy_hi, *ixy_lo);

    #define DISP() (int8_t)read8(REG_PC++)

    switch (opcode)
    {
        case 0x36: // addr disp is stored first, then imm value
        {
            const uint16_t addr = pair + DISP();
            const uint8_t value = read8(REG_PC++);
            write8(addr, value);
        } break;

        case 0x64: *ixy_hi = *ixy_hi; break;
        case 0x65: *ixy_hi = *ixy_lo; break;
        case 0x6C: *ixy_lo = *ixy_hi; break;
        case 0x6D: *ixy_lo = *ixy_lo; break;

        case 0x60: case 0x61: case 0x62: case 0x63: case 0x67:
            *ixy_hi = get_r8(sms, opcode);
            break;

        case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6F:
            *ixy_lo = get_r8(sms, opcode);
            break;

        case 0x46: case 0x4E: case 0x56: case 0x5E:
        case 0x66: case 0x6E: case 0x70: case 0x71:
        case 0x72: case 0x73: case 0x74: case 0x75:
        case 0x7E:
            write8(pair + DISP(), get_r8(sms, opcode));
            break;

        case 0xCB: execute_CB_IXIY(sms, pair); return;

        default:
            SMS_log_fatal("UNK OP: 0xFD%02X\n", opcode);
            break;
    }

    #undef DISP
}

static void execute_ED(struct SMS_Core* sms)
{
	const uint8_t opcode = SMS_read8(sms, REG_PC++);

	switch (opcode)
	{
        case 0x42: SBC_hl(sms, REG_BC); break;
        case 0x52: SBC_hl(sms, REG_DE); break;
        case 0x62: SBC_hl(sms, REG_HL); break;
        case 0x72: SBC_hl(sms, REG_SP); break;

        case 0x4B: ADC_hl(sms, REG_BC); break;
        case 0x5B: ADC_hl(sms, REG_DE); break;
        case 0x6B: ADC_hl(sms, REG_HL); break;
        case 0x7B: ADC_hl(sms, REG_SP); break;

        case 0x44: case 0x54: case 0x64: case 0x74:
        case 0x4C: case 0x5C: case 0x6C: case 0x7C:
            NEG(sms);
            break;

		case 0x46: case 0x66:
			IMM_set(sms, 0);
			break;

		case 0x56: case 0x76:
			IMM_set(sms, 1);
			break;
			
		case 0x5E: case 0x7E:
			IMM_set(sms, 2);
			break;

        case 0xB0:
            LDIR(sms);
            break;
			
		default:
			SMS_log_fatal("UNK OP: 0xED%02X\n", opcode);
			break;
	}
}

static void execute(struct SMS_Core* sms)
{
	const uint8_t opcode = SMS_read8(sms, REG_PC++);

	switch (opcode)
	{
		case 0x00: break; // nop
		
        case 0x07: RLCA(sms); break;
        case 0x0F: RRCA(sms); break;
        case 0x17: RLA(sms); break;
        case 0x1F: RRA(sms); break;

		case 0x04: case 0x0C: case 0x14: case 0x1C:
		case 0x24: case 0x2C: case 0x34: case 0x3C:
			INC_r8(sms, opcode);
			break;

		case 0x05: case 0x0D: case 0x15: case 0x1D:
		case 0x25: case 0x2D: case 0x35: case 0x3D:
			DEC_r8(sms, opcode);
			break;

		case 0x03: case 0x13: case 0x23: case 0x33:
			INC_r16(sms, opcode);
			break;

		case 0x0B: case 0x1B: case 0x2B: case 0x3B:
			DEC_r16(sms, opcode);
			break;

		case 0x09: case 0x19: case 0x29: case 0x39:
			ADD_HL(sms, opcode);
			break;

		case 0x01: case 0x11: case 0x21: case 0x31:
			LD_16(sms, opcode);
			break;

        case 0x02: LD_r16_a(sms, REG_BC); break;
        case 0x12: LD_r16_a(sms, REG_DE); break;
        case 0x0A: LD_a_r16(sms, REG_BC); break;
        case 0x1A: LD_a_r16(sms, REG_BC); break;
        case 0x22: LD_imm_hl(sms); break;
        case 0x2A: LD_hl_imm(sms); break;
        case 0x32: LD_imm_a(sms); break;
        case 0x3A: LD_a_imm(sms); break;
        case 0xF9: LD_sp_hl(sms); break;

		case 0x06: case 0x0E: case 0x16: case 0x1E:
		case 0x26: case 0x2E: case 0x36: case 0x3E:
			LD_r_u8(sms, opcode);
			break;

		case 0x41: case 0x42: case 0x43: case 0x44:
		case 0x45: case 0x46: case 0x47: case 0x48:
		case 0x4A: case 0x4B: case 0x4C: case 0x4D:
		case 0x4E: case 0x4F: case 0x50: case 0x51:
		case 0x53: case 0x54: case 0x55: case 0x56:
		case 0x57: case 0x58: case 0x59: case 0x5A:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x65: case 0x66: case 0x67: case 0x68:
		case 0x69: case 0x6A: case 0x6B: case 0x6C:
		case 0x6E: case 0x6F: case 0x70: case 0x71:
		case 0x72: case 0x73: case 0x74: case 0x75:
		case 0x77: case 0x78: case 0x79: case 0x7A:
		case 0x7B: case 0x7C: case 0x7D: case 0x7E:
			LD_rr(sms, opcode);
			break;

		case 0x40: break; // nop LD b,b
		case 0x49: break; // nop LD c,c
		case 0x52: break; // nop LD d,d
		case 0x5B: break; // nop LD e,e
		case 0x64: break; // nop LD h,h
		case 0x6D: break; // nop LD l,l
		case 0x7F: break; // nop LD a,a

		// case 0x76: HALT(sms); break;

		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
			ADD_r(sms, opcode);
			break;

		case 0x88: case 0x89: case 0x8A: case 0x8B:
		case 0x8C: case 0x8D: case 0x8E: case 0x8F:
			ADC_r(sms, opcode);
			break;

		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
			SUB_r(sms, opcode);
			break;

		case 0x98: case 0x99: case 0x9A: case 0x9B:
		case 0x9C: case 0x9D: case 0x9E: case 0x9F:
			SBC_r(sms, opcode);
			break;

		case 0xA0: case 0xA1: case 0xA2: case 0xA3:
		case 0xA4: case 0xA5: case 0xA6: case 0xA7:
			AND_r(sms, opcode);
			break;

		case 0xA8: case 0xA9: case 0xAA: case 0xAB:
		case 0xAC: case 0xAD: case 0xAE: case 0xAF:
			XOR_r(sms, opcode);
			break;

		case 0xB0: case 0xB1: case 0xB2: case 0xB3:
		case 0xB4: case 0xB5: case 0xB6: case 0xB7:
			OR_r(sms, opcode);
			break;

		case 0xB8: case 0xB9: case 0xBA: case 0xBB:
		case 0xBC: case 0xBD: case 0xBE: case 0xBF:
			CP_r(sms, opcode);
			break;

		case 0xC6: ADD_imm(sms); break;
		case 0xCE: ADC_imm(sms); break;
		case 0xD6: SUB_imm(sms); break;
		case 0xDE: SBC_imm(sms); break;
		case 0xE6: AND_imm(sms); break;
		case 0xEE: XOR_imm(sms); break;
		case 0xF6: OR_imm(sms); break;
		case 0xFE: CP_imm(sms); break;

		case 0x10: DJNZ(sms); break;
		case 0x18: JR(sms); break;
		case 0xC3: JP(sms); break;
		case 0xC9: RET(sms); break;
		case 0xCD: CALL(sms); break;

		case 0xC1: SET_REG_BC(POP(sms)); break;
		case 0xD1: SET_REG_DE(POP(sms)); break;
		case 0xE1: SET_REG_HL(POP(sms)); break;
		case 0xF1: SET_REG_AF(POP(sms)); break;

		case 0xC5: PUSH(sms, REG_BC); break;
		case 0xD5: PUSH(sms, REG_DE); break;
		case 0xE5: PUSH(sms, REG_HL); break;
		case 0xF5: PUSH(sms, REG_AF); break;

		case 0x20: case 0x30: case 0x28: case 0x38:
			JR_cc(sms, opcode);
			break;

		case 0xC0: case 0xD0: case 0xE0: case 0xF0:
		case 0xC8: case 0xD8: case 0xE8: case 0xF8:
			RET_cc(sms, opcode);
			break;

		case 0xC2: case 0xD2: case 0xE2: case 0xF2:
		case 0xCA: case 0xDA: case 0xEA: case 0xFA:
			JP_cc(sms, opcode);
			break;

		case 0xC4: case 0xD4: case 0xE4: case 0xF4:
		case 0xCC: case 0xDC: case 0xEC: case 0xFC:
			CALL_cc(sms, opcode);
			break;

		case 0xC7: case 0xCF: case 0xD7: case 0xDF:
		case 0xE7: case 0xEF: case 0xF7: case 0xFF:
			RST(sms, opcode & 0x38);
			break;

		case 0x08: EX_af_af(sms); break;
		case 0xD9: EXX(sms); break;
		case 0xE3: EX_sp_hl(sms); break;
		case 0xEB: EX_de_hl(sms); break;

		case 0xD3: OUT_imm(sms); break;
		case 0xDB: IN_imm(sms); break;

		case 0xF3: DI(sms); break;
		case 0xFB: EI(sms); break;

		// case 0xDB: break;

		case 0xCB: execute_CB(sms); return;
		case 0xDD: execute_IXIY(sms, &REG_IXH, &REG_IXL); return;
		case 0xED: execute_ED(sms); return;
		case 0xFD: execute_IXIY(sms, &REG_IYH, &REG_IYL); return;

		default:
			SMS_log_fatal("UNK OP: 0x%02X\n", opcode);
			break;
	}
}

void Z80_run(struct SMS_Core* sms)
{
	execute(sms);
}
