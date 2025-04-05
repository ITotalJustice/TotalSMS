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

// Generic helper definitions for shared library support
// SEE: https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
  #define SMS_HELPER_DLL_IMPORT __declspec(dllimport)
  #define SMS_HELPER_DLL_EXPORT __declspec(dllexport)
  #define SMS_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define SMS_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define SMS_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define SMS_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SMS_HELPER_DLL_IMPORT
    #define SMS_HELPER_DLL_EXPORT
    #define SMS_HELPER_DLL_LOCAL
  #endif
#endif

#ifdef SMS_DLL /* defined if SMS is compiled as a DLL */
  #ifdef SMS_DLL_EXPORTS /* defined if we are building the SMS DLL (instead of using it) */
    #define SMS_API SMS_HELPER_DLL_EXPORT
  #else
    #define SMS_API SMS_HELPER_DLL_IMPORT
  #endif /* SMS_DLL_EXPORTS */
  #define SMS_LOCAL SMS_HELPER_DLL_LOCAL
#else /* SMS_DLL is not defined: this means SMS is a static lib. */
  #define SMS_API
  #define SMS_LOCAL
#endif /* SMS_DLL */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <scheduler.h>
#include <sn76489.h>


// fwd
struct SMS_Ports;
struct SMS_MemoryControlRegister;
struct SMS_Core;


// callback types
typedef void (*sms_apu_callback_t)(void* user, int16_t* samples, uint32_t size);
typedef void (*sms_vblank_callback_t)(void* user);
typedef uint32_t (*sms_colour_callback_t)(void* user, uint8_t r, uint8_t g, uint8_t b);
typedef void (*sms_input_callback_t)(void* user, int port);

enum
{
    SMS_SCREEN_WIDTH = 256,
    SMS_SCREEN_HEIGHT = 192, // actually 224 (240 but nothing used it)

    GG_SCREEN_WIDTH = 160,
    GG_SCREEN_HEIGHT = 144,

    SMS_ROM_SIZE_MAX = 1024 * 512, // 512KiB
    SMS_SRAM_SIZE_MAX = 1024 * 16 * 2, // 2 banks of 16kib

    // default max sprites
    SMS_MODE1_MAX_SPRITES = 4,
    SMS_MODE4_MAX_SPRITES = 8,
};

enum SMS_Region
{
    SMS_Region_NTSC,
    SMS_Region_PAL,
};

enum SMS_Console
{
    //  Europe, Australia, USA, Brazil
    SMS_Console_EXPORT,
    // Japan, Korea
    SMS_Console_JAPANESE,
};

enum SMS_System
{
    SMS_System_SMS,
    SMS_System_GG,
    SMS_System_SG1000,
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

enum Z80_ExecutionMode {
    Z80_ExecutionMode_RUNNING,
    Z80_ExecutionMode_HALT,
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

    enum Z80_ExecutionMode execution_mode;
    // bool halt;
};

enum SMS_MapperType
{
    MAPPER_TYPE_SEGA, // nomal sega mapper (can have sram)
    MAPPER_TYPE_CODEMASTERS,
    MAPPER_TYPE_KOREAN,
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
    uint8_t fffc;
    uint8_t fffd;
    uint8_t fffe;
    uint8_t ffff;
};

struct SMS_CodemastersMapper
{
    uint8_t slot[3];
    bool ram_mapped; // ernie els golf features 8k on cart ram
};

struct SMS_KoreanMapper
{
    uint8_t slot2;
};

struct SMS_Cart
{
    enum SMS_MapperType mapper_type;

    union
    {
        struct SMS_SegaMapper sega;
        struct SMS_CodemastersMapper codemasters;
        struct SMS_KoreanMapper korean;
    } mappers;

    // some games have 8-16-32KiB ram
    uint8_t ram[2][1024 * 16];

    uint8_t max_bank_mask;
    bool sram_used; // set when game uses sram at any point
    bool sram_dirty; // set whilst sram is mapped.
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

// see vdp.c for details
enum VdpState
{
    VdpState_ACTIVE,
    VdpState_BLANKING,
};

struct VdpSpriteEntry
{
    int16_t y;
    int16_t x;
    // below are not used in mode 4.
    uint8_t tile_num;
    uint8_t colour;
};

struct SMS_Vdp
{
    // this is used for vram r/w and cram writes.
    uint16_t addr;
    enum VDP_Code code;

    // we allow for more sprites than normal
    // as this reduces flicker in most games.
    struct VdpSpriteEntry sprites[16];
    // max mode4 = 8, otherwise max = 4.
    uint8_t sprites_count;

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

    uint16_t hcount;
    uint16_t vcount;

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

    // (all of below is cleared upon reading stat)
    // set on vblank
    bool frame_interrupt_pending;
    // set on line counter underflow
    bool line_interrupt_pending;
    // set when there's more than 8(sms)/4(sg) sprites on a line
    bool sprite_overflow;
    // set when a sprite collides
    bool sprite_collision;
    // 5th sprite number sg-1000
    uint8_t fifth_sprite_num;

    enum VdpState state;
    bool nmi_pending;
};

// the actual impl is there are two ports (joy1/joy2)
// i expose the both ports as a single u16 to simplify the frontend code.
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

struct SMS_Ports
{
    uint8_t gg_regs[7];
    uint8_t a;
    uint8_t b;
};

struct SMS_MemoryControlRegister
{
    bool exp_slot_disable;
    bool cart_slot_disable;
    bool card_slot_disable;
    bool work_ram_disable;
    bool bios_rom_disable;
    bool io_chip_disable;
};

struct SMS_StateConfig
{
    bool fast;
    bool include_psg_blip;
};

struct SMS_Core
{
    // mapped every 0x400 due to how sega mapper works with the first
    // page being fixed (0x400 in size).
    const uint8_t* rmap[0x10000 / 0x400]; // 64
    uint8_t* wmap[0x10000 / 0x400]; // 64

    struct Scheduler scheduler;
    struct Z80 cpu;
    struct SMS_Vdp vdp;
    Sn76489* psg;
    struct SMS_Cart cart;
    struct SMS_Ports port;
    struct SMS_MemoryControlRegister memory_control;
    uint8_t system_ram[0x2000];

    uint32_t crc;
    enum SMS_System system;
    enum SMS_Region region;
    enum SMS_Console console;

    const uint8_t* rom;
    size_t rom_size;
    uint32_t rom_mask;

    const uint8_t* bios;
    size_t bios_size;

    void* pixels;
    uint16_t stride;
    uint8_t bpp;
    uint8_t mode1_max_spirtes;
    uint8_t mode4_max_spirtes;
    bool skip_audio;
    bool skip_frame;

    bool frame_end;

    uint32_t builtin_palette[16];

    sms_vblank_callback_t vblank_callback;
    sms_colour_callback_t colour_callback;
    sms_apu_callback_t apu_callback;
    sms_input_callback_t input_callback;
    void* userdata;

    // set by frontend via SMS_set_apu_callback().
    int16_t* samples;
    // number of stereo samples.
    size_t sample_size;
    // set by frontend
    uint32_t sample_freq;
    // set by the frontend, 0 - 2.0
    float volume[4];
    // master volume override, 0 - 2.0
    float master_volume;
};

#ifdef __cplusplus
}
#endif
