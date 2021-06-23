#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SMS_DEBUG
    #define SMS_DEBUG 0
#endif

#ifndef SMS_SINGLE_FILE
    #define SMS_SINGLE_FILE 0
#endif

#if defined _WIN32 || defined __CYGWIN__
    #ifdef BUILDING_LIB
        #define SMSAPI __declspec(dllexport)
    #else
        #define SMSAPI __declspec(dllimport)
    #endif
#else
    #ifdef BUILDING_LIB
        #define SMSAPI __attribute__ ((visibility ("default")))
    #else
        #define SMSAPI
    #endif
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// fwd
struct SMS_Ports;
struct SMS_ApuCallbackData;
struct SMS_MemoryControlRegister;
struct SMS_Core;


// callback types
typedef void (*sms_apu_callback_t)(void* user, struct SMS_ApuCallbackData* data);
typedef void (*sms_vblank_callback_t)(void* user);
typedef uint32_t (*sms_colour_callback_t)(void* user, uint8_t colour_b2g2r2);


enum
{
    SMS_SCREEN_WIDTH = 342,
    SMS_SCREEN_HEIGHT = 262,

    SMS_ROM_SIZE_MAX = 0x80000, // 512KiB
};

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

struct Z80
{
    uint16_t cycles;

    // [special purpose registers]
    uint16_t PC; // program counter
    uint16_t SP; // stack pointer

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
    bool ei_delay; // like the gb, ei is delayed by 1 instructions
    bool halt;

    bool interrupt_requested;
};

enum SMS_MapperType
{
    MAPPER_TYPE_NONE,
    MAPPER_TYPE_SEGA,
};

struct SMS_SegaMapper
{
    const uint8_t* banks[48]; // mapped every 0x400

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
    const uint8_t* banks[3]; // mapped every 0x4000
};

struct SMS_Cart
{
    enum SMS_MapperType mapper_type;

    union
    {
        struct SMS_SegaMapper sega;
        struct SMS_CodemastersMapper codemasters;
    } mappers;
};

struct SMS_RomHeader
{
    uint8_t magic[0x8];
    uint16_t checksum;
    uint32_t prod_code;
    uint8_t version;
    uint8_t region_code;
    uint8_t rom_size;
};

enum VDP_Code
{
    VDP_CODE_VRAM_WRITE_LOAD,
    VDP_CODE_VRAM_WRITE,
    VDP_CODE_REG_WRITE,
    VDP_CODE_CRAM_WRITE,
};

struct SMS_Vdp
{
    // this is used for vram r/w and cram writes.
    uint16_t addr;
    // see [enum VDP_Code] 
    uint8_t code;

    uint8_t vram[1024 * 16];

    // bg can use either palette, while sprites can only use
    // the second half of the cram.
    uint8_t cram[32];

    // the actual colour set to the pixels
    uint32_t colour[32];

    // not sure if i should split this into structs...
    uint8_t registers[0x10];

    // vertical scroll is updated when the display is not active,
    // not when the register is updated!
    uint8_t vertical_scroll;

    uint16_t hcount;
    uint16_t vcount;

    // this differ from above in that this is what will be read on the port.
    // due to the fact that scanlines can be 262 or 312, the value would
    // would eventually overflow, so scanline 256 would read as 0!
    // internally this does not actually wrap around, instead, at set
    // values (differes between ntsc and pal), it will jump back to a
    // previous value, for example, on ntsc, it'll jump from value
    // 218 back to 213, though i am unsure if it keeps jumping...
    uint8_t vcount_port;

    // used for interrupts, reloaded at 0
    uint8_t line_counter;

    // reads are buffered
    uint8_t buffer_read_data;

    // it takes 2 writes to control port to form the control word
    // this can be used to set the addr, vdp reg or set writes
    // to be made to cram
    uint16_t control_word;

    // set if already have lo byte
    bool control_latch;

    // idk...its cleared on control port read
    bool frame_interrupt_pending;
    // set when there's more than 8 sprites on a line
    bool sprite_overflow;
    // set when a sprite collides
    bool sprite_collision;
};

enum SMS_PortA
{
    JOY1_UP_BUTTON      = 1 << 0,
    JOY1_DOWN_BUTTON    = 1 << 1,
    JOY1_LEFT_BUTTON    = 1 << 2,
    JOY1_RIGHT_BUTTON   = 1 << 3,
    JOY1_A_BUTTON       = 1 << 4,
    JOY1_B_BUTTON       = 1 << 5,
    JOY2_UP_BUTTON      = 1 << 6,
    JOY2_DOWN_BUTTON    = 1 << 7,
};

enum SMS_PortB
{
    JOY2_LEFT_BUTTON    = 1 << 0,
    JOY2_RIGHT_BUTTON   = 1 << 1,
    JOY2_A_BUTTON       = 1 << 2,
    JOY2_B_BUTTON       = 1 << 3,
    RESET_BUTTON        = 1 << 4,
    PAUSE_BUTTON        = 1 << 5,
};

struct SMS_Ports
{
    uint8_t a;
    uint8_t b;
};

struct SMS_ApuCallbackData
{
    int8_t tone0;
    int8_t tone1;
    int8_t tone2;
    int8_t noise;
};

struct SN76489
{
    struct
    {
        int32_t counter; // 10-bits
        uint16_t tone; // 10-bits
    } tone[3];

    struct
    {
        int32_t counter; // 10-bits
        uint16_t lfsr; // can be either 16-bit or 15-bit...
        uint8_t mode; // 1-bits
        uint8_t shift_rate; // 2-bits
        bool flip_flop;
        // enable to have better sounding drums in most games!
        bool better_drums;
    } noise;

    uint8_t volume[4];
    int8_t polarity[4];

    // which of the 4 channels are latched.
    uint8_t latched_channel;
    // vol or tone (or mode + shift instead of tone for noise).
    uint8_t latched_type;
};

struct SMS_MemoryControlRegister
{
    bool exp_slot_enable;
    bool cart_slot_enable;
    bool card_slot_disable;
    bool work_ram_disable;
    bool bios_rom_disable;
    bool io_chip_disable;
};

struct SMS_Core
{
    struct Z80 cpu;
    struct SMS_Vdp vdp;
    struct SN76489 apu;
    struct SMS_Cart cart;
    struct SMS_Ports port;
    struct SMS_MemoryControlRegister memory_control;
    uint8_t system_ram[0x2000];

    const uint8_t* rom;
    size_t rom_size;
    uint32_t rom_mask;

    void* pixels;
    uint32_t pixels_stride;
    uint8_t bpp;

    sms_vblank_callback_t vblank_callback;
    void* vblank_callback_user;

    sms_colour_callback_t colour_callback;
    void* colour_callback_user;

    sms_apu_callback_t apu_callback;
    void* apu_callback_user;

    uint32_t apu_callback_freq;
    int32_t apu_callback_counter;
};

struct SMS_State
{
    uint16_t magic;
    uint16_t reserved;

    struct Z80 cpu;
    struct SMS_Vdp vdp;
    struct SN76489 apu;
    struct SMS_Cart cart;
    struct SMS_MemoryControlRegister memory_control;
    uint8_t system_ram[0x2000];
};

#ifdef __cplusplus
}
#endif
