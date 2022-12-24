#pragma once

// notes:
// - bitfields
//   - i have choses to use bitfields for bools ONLY
//   - this is purely for struct packing, saving several bytes
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SMS_DEBUG
    #define SMS_DEBUG 0
#endif

#ifndef SMS_SINGLE_FILE
    #define SMS_SINGLE_FILE 0
#endif


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


// fwd
struct SMS_Ports;
struct SMS_ApuSample;
struct SMS_MemoryControlRegister;
struct SMS_Core;


// callback types
typedef void (*sms_apu_callback_t)(void* user, struct SMS_ApuSample* samples, uint32_t size);
typedef void (*sms_vblank_callback_t)(void* user);
typedef uint32_t (*sms_colour_callback_t)(void* user, uint8_t r, uint8_t g, uint8_t b);


enum
{
    SMS_SCREEN_WIDTH = 256,
    SMS_SCREEN_HEIGHT = 192, // actually 224 (240 but nothing used it)

    GG_SCREEN_WIDTH = 160,
    GG_SCREEN_HEIGHT = 144,

    SMS_ROM_SIZE_MAX = 1024 * 512, // 512KiB
    SMS_SRAM_SIZE_MAX = 1024 * 16 * 2, // 2 banks of 16kib

    // this value was taken for sms power docs
    SMS_CPU_CLOCK = 3579545,

    SMS_CYCLES_PER_FRAME = SMS_CPU_CLOCK / 60,
};

enum SMS_System
{
    SMS_System_SMS,
    SMS_System_GG,
    SMS_System_SG1000,
};

// this is currently unused!
// enum SMS_SystemMode
// {
//     SMS_SystemMode_SMS1,
//     SMS_SystemMode_SMS2,

//     SMS_SystemMode_GG_SMS, // GG sys, in sms mode
//     SMS_SystemMode_GG, // GG sys in GG mode
// };

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
        bool C : 1;
        bool N : 1;
        bool P : 1;
        bool B3 : 1; // bit3
        bool H : 1;
        bool B5 : 1; // bit5
        bool Z : 1;
        bool S : 1;
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
    bool IFF1 : 1;
    bool IFF2 : 1;
    bool ei_delay : 1; // like the gb, ei is delayed by 1 instructions
    bool halt : 1;

    bool interrupt_requested : 1;
};

enum SMS_MapperType
{
    MAPPER_TYPE_SEGA, // nomal sega mapper (can have sram)
    MAPPER_TYPE_CODEMASTERS, // todo:
    MAPPER_TYPE_NONE, // 8K - 48K
    // https://segaretro.org/8kB_RAM_Adapter
    // https://www.smspower.org/forums/13579-Taiwan8KBRAMAdapterForPlayingMSXPortsOnSG1000II
    // thanks to bock for the info about the 2 types of mappings
    // thanks to calindro for letting me know of dahjee adapter :)
    MAPPER_TYPE_DAHJEE_A, // extra 8K ram 0x2000-0x3FFF, normal 1K at 0xC000-0xFFFF
    MAPPER_TYPE_DAHJEE_B, // extra 8K ram 0xC000-0xFFFF

    // special case mappers
    MAPPER_TYPE_THE_CASTLE,
    MAPPER_TYPE_OTHELLO,
};

struct SMS_SegaMapper
{
    struct // control
    {
        bool rom_write_enable;
        bool ram_enable_c0000;
        bool ram_enable_80000;
        bool ram_bank_select;
        uint8_t bank_shift;
    } fffc;

    uint8_t fffd;
    uint8_t fffe;
    uint8_t ffff;
};

struct SMS_CodemastersMapper
{
    uint8_t slot[3];
    bool ram_mapped; // ernie els golf features 8k on cart ram
};

struct SMS_Cart
{
    enum SMS_MapperType mapper_type;

    union
    {
        struct SMS_SegaMapper sega;
        struct SMS_CodemastersMapper codemasters;
    } mappers;

    // some games have 8-16-32KiB ram
    uint8_t ram[2][1024 * 16];

    uint8_t max_bank_mask;
    bool sram_used; // set when game uses sram at any point
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

struct CachedTile
{
    uint16_t palette_index : 9;
    bool priority : 1;
    bool palette_select : 1;
    bool vertical_flip : 1;
    bool horizontal_flip : 1;
    bool dirty : 1;
};

// basically packed u32, easier for me to understand
struct CachedPalette
{
    uint32_t flipped;
    uint32_t normal;
};

struct SMS_Vdp
{
    // this is used for vram r/w and cram writes.
    uint16_t addr;
    enum VDP_Code code;

    uint8_t vram[1024 * 16];
    bool dirty_vram[(1024 * 16) / 4];
    struct CachedPalette cached_palette[(1024 * 16) / 4];

    // bg can use either palette, while sprites can only use
    // the second half of the cram.
    uint8_t cram[64];

    // writes to even addresses are latched!
    uint8_t cram_gg_latch;

    // set when cram value changes, the colour callback is then called
    // during rendering of the line.
    bool dirty_cram[64];
    // indicates where the loop should start and end
    uint8_t dirty_cram_min;
    uint8_t dirty_cram_max;

    // the actual colour set to the pixels
    uint32_t colour[32];

    // 16 registers, not all are useable
    uint8_t registers[0x10];

    // vertical scroll is updated when the display is not active,
    // not when the register is updated!
    uint8_t vertical_scroll;

    int16_t cycles;
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
    bool control_latch : 1;

    // (all of below is cleared upon reading stat)
    // set on vblank
    bool frame_interrupt_pending : 1;
    // set on line counter underflow
    bool line_interrupt_pending : 1;
    // set when there's more than 8(sms)/4(sg) sprites on a line
    bool sprite_overflow : 1;
    // set when a sprite collides
    bool sprite_collision : 1;
    // 5th sprite number sg-1000
    uint8_t fifth_sprite_num : 5;
};

enum SMS_Button
{
    SMS_Button_JOY1_UP      = 1 << 0,
    SMS_Button_JOY1_DOWN    = 1 << 1,
    SMS_Button_JOY1_LEFT    = 1 << 2,
    SMS_Button_JOY1_RIGHT   = 1 << 3,
    SMS_Button_JOY1_A       = 1 << 4,
    SMS_Button_JOY1_B       = 1 << 5,
    SMS_Button_JOY2_UP      = 1 << 6,
    SMS_Button_JOY2_DOWN    = 1 << 7,

    SMS_Button_JOY2_LEFT    = 1 << 8,
    SMS_Button_JOY2_RIGHT   = 1 << 9,
    SMS_Button_JOY2_A       = 1 << 10,
    SMS_Button_JOY2_B       = 1 << 11,
    SMS_Button_RESET        = 1 << 12,
    SMS_Button_PAUSE        = 1 << 13,
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
    uint8_t gg_regs[7];
    uint8_t a;
    uint8_t b;
};

struct SMS_ApuSample
{
    uint8_t tone0[2];
    uint8_t tone1[2];
    uint8_t tone2[2];
    uint8_t noise[2];
};

struct SMS_Psg
{
    uint32_t cycles; // elapsed cycles since last psg_sync()

    struct
    {
        int16_t counter; // 10-bits
        uint16_t tone; // 10-bits
    } tone[3];

    struct
    {
        int16_t counter; // 10-bits
        uint16_t lfsr; // can be either 16-bit or 15-bit...
        uint8_t mode; // 1-bits
        uint8_t shift_rate; // 2-bits
        bool flip_flop;
    } noise;

    uint8_t volume[4];
    uint8_t polarity[4];

    // which of the 4 channels are latched.
    uint8_t latched_channel;
    // vol or tone (or mode + shift instead of tone for noise).
    uint8_t latched_type;

    // GG has stereo switches for each channel
    bool channel_enable[4][2];
};

struct SMS_MemoryControlRegister
{
    bool exp_slot_disable : 1;
    bool cart_slot_disable : 1;
    bool card_slot_disable : 1;
    bool work_ram_disable : 1;
    bool bios_rom_disable : 1;
    bool io_chip_disable : 1;
};

struct SMS_Core
{
    // mapped every 0x400 due to how sega mapper works with the first
    // page being fixed (0x400 in size).
    const uint8_t* rmap[0x10000 / 0x400]; // 64
    uint8_t* wmap[0x10000 / 0x400]; // 64

    struct Z80 cpu;
    struct SMS_Vdp vdp;
    struct SMS_Psg psg;
    struct SMS_Cart cart;
    struct SMS_Ports port;
    struct SMS_MemoryControlRegister memory_control;
    uint8_t system_ram[0x2000];

    uint32_t crc;
    enum SMS_System system;

    const uint8_t* rom;
    size_t rom_size;
    uint32_t rom_mask;

    const uint8_t* bios;
    size_t bios_size;

    void* pixels;
    uint16_t pitch;
    uint8_t bpp;
    bool skip_frame;

    sms_vblank_callback_t vblank_callback;
    sms_colour_callback_t colour_callback;
    sms_apu_callback_t apu_callback;
    void* userdata;

    struct SMS_ApuSample* apu_samples; // sample buffer
    uint32_t apu_sample_size; // number of samples
    uint32_t apu_sample_index; // index into the buffer
    uint32_t apu_callback_freq; // sample rate
    uint32_t apu_callback_counter; // how many cpu cycles until sample

    // enable to have better sounding drums in most games!
    bool better_drums;
};

struct SMS_State
{
    struct SMS_StateHeader
    {
        uint16_t magic;
        uint16_t version;
        uint32_t crc;
        // uint32_t size;
    } header;

    struct Z80 cpu;
    struct SMS_Vdp vdp;
    struct SMS_Psg psg;
    struct SMS_Cart cart;
    struct SMS_MemoryControlRegister memory_control;
    uint8_t system_ram[0x2000];
};

#ifdef __cplusplus
}
#endif
