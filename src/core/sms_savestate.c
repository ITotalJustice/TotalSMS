#include "sms_internal.h"
#include "sms.h"
#include <stdint.h>
#include <string.h>

struct StateZ80
{
    uint32_t PC;
    uint32_t SP;

    uint32_t IXL;
    uint32_t IXH;
    uint32_t IYL;
    uint32_t IYH;

    uint32_t I; // interrupt vector
    uint32_t R; // memory refresh

    struct RegisterSet
    {
        uint32_t B;
        uint32_t C;
        uint32_t D;
        uint32_t E;
        uint32_t F;
        uint32_t H;
        uint32_t L;
        uint32_t A;
    } register_set[2];

    uint32_t IFF1;
    uint32_t IFF2;
    uint32_t ei_delay;
    uint32_t execution_mode;

    uint32_t reserved[4];
};

struct StateVdpSpriteEntry
{
    uint32_t y;
    uint32_t x;
    uint32_t tile_num;
    uint32_t colour;
};

struct StateVdp
{
    uint8_t registers[0x10];
    uint8_t cram[64];
    uint8_t vram[1024 * 16];

    struct StateVdpSpriteEntry sprites[16];
    uint32_t sprites_count;

    uint32_t addr;
    uint32_t code;
    uint32_t cram_gg_latch;
    uint32_t vertical_scroll;
    uint32_t hcount;
    uint32_t vcount;
    uint32_t line_counter;
    uint32_t buffer_read_data;
    uint32_t control_word;
    uint32_t control_latch;
    uint32_t frame_interrupt_pending;
    uint32_t line_interrupt_pending;
    uint32_t sprite_overflow;
    uint32_t sprite_collision;
    uint32_t fifth_sprite_num;
    uint32_t state;
    uint32_t nmi_pending;

    uint32_t _padding[16];
};

struct StateMappers
{
    uint8_t data[16];
};

struct StateCart
{
    uint8_t ram[2][1024 * 16];
    struct StateMappers cart_mapper;
    struct StateMappers bios_mapper;
    uint32_t sram_used;
    uint8_t gg_regs[8];
    uint32_t _padding[14];
};

struct StateMemoryControlRegister
{
    uint32_t exp_slot_disable;
    uint32_t cart_slot_disable;
    uint32_t card_slot_disable;
    uint32_t work_ram_disable;
    uint32_t bios_rom_disable;
    uint32_t io_chip_disable;
};

struct StateMeta
{
    uint32_t magic;
    char name[32]; // NULL terminated string
    uint32_t emu_version;
    uint32_t state_version_major; // breaking change
    uint32_t state_version_minor; // not a breaking change
    uint32_t state_size;
    uint32_t rom_crc32;
    uint32_t region;
    uint32_t console;
    uint32_t _padding[14];
};

struct Rts
{
    struct StateMeta meta;
    struct StateZ80 z80;
    struct StateVdp vdp;
    struct StateCart cart;
    struct StateMemoryControlRegister memory_control;
    uint8_t psg[512];
    uint8_t scheduler[64];
    uint8_t system_ram[0x2000];
};

enum { STATE_MAGIC = 0x5E6A0535 };
enum { STATE_VERSION_MAJOR = 2 };
enum { STATE_VERSION_MINOR = 4 };
enum { STATE_VERSION = (STATE_VERSION_MAJOR << 16) | STATE_VERSION_MINOR };

sms_static_assert(sizeof(struct Rts) == 58764, "state size is broken");
sms_static_assert(sizeof(struct SMS_Mappers) <= sizeof(struct StateMappers), "state mapper size is broken");

static const struct SMS_StateConfig DEFAULT_CONFIG = {
    .fast = false,
    .include_psg_blip = false,
};

static size_t state_create_version(uint32_t major, uint32_t minor)
{
    return (major << 16) | minor;
}

static bool state_version_atleast(const struct Rts* rts, uint32_t major, uint32_t minor)
{
    const size_t rts_version = state_create_version(rts->meta.state_version_major, rts->meta.state_version_minor);
    return rts_version >= state_create_version(major, minor);
}

static const struct SMS_StateConfig* state_get_config(const struct SMS_StateConfig* config)
{
    return config ? config : &DEFAULT_CONFIG;
}

size_t SMS_get_state_size(const struct SMS_Core* sms, const struct SMS_StateConfig* config)
{
    config = state_get_config(config);
    size_t size = sizeof(struct Rts);

    if (config->include_psg_blip)
    {
        size += psg_state_size(sms->psg, config->include_psg_blip);
    }

    return size;
}

bool SMS_savestate(const struct SMS_Core* sms, void* data, size_t size, const struct SMS_StateConfig* config)
{
    config = state_get_config(config);

    const size_t state_size = SMS_get_state_size(sms, config);
    if (size < state_size)
    {
        return false;
    }

    struct Rts* rts = data;
    if (!config->fast)
    {
        memset(rts, 0, sizeof(*rts));
    }

    /* ---META---*/
    {
        rts->meta.magic = STATE_MAGIC;
        strcpy(rts->meta.name, "TotalSMS");
        rts->meta.state_version_major = STATE_VERSION_MAJOR;
        rts->meta.state_version_minor = STATE_VERSION_MINOR;
        rts->meta.state_size = state_size;
        rts->meta.rom_crc32 = sms->crc;
        rts->meta.region = sms->region;
        rts->meta.console = sms->console;
    }

    /* ---Z80---*/
    {
        rts->z80.PC = sms->cpu.PC;
        rts->z80.SP = sms->cpu.SP;

        rts->z80.IXL = sms->cpu.IXL;
        rts->z80.IXH = sms->cpu.IXH;
        rts->z80.IYL = sms->cpu.IYL;
        rts->z80.IYH = sms->cpu.IYH;

        rts->z80.I = sms->cpu.I; // interrupt vector
        rts->z80.R = sms->cpu.R; // memory refresh

        rts->z80.register_set[0].B = sms->cpu.main.B;
        rts->z80.register_set[0].C = sms->cpu.main.C;
        rts->z80.register_set[0].D = sms->cpu.main.D;
        rts->z80.register_set[0].E = sms->cpu.main.E;
        rts->z80.register_set[0].F = z80_get_reg_main(sms, 0x6);
        rts->z80.register_set[0].H = sms->cpu.main.H;
        rts->z80.register_set[0].L = sms->cpu.main.L;
        rts->z80.register_set[0].A = sms->cpu.main.A;

        rts->z80.register_set[1].B = sms->cpu.alt.B;
        rts->z80.register_set[1].C = sms->cpu.alt.C;
        rts->z80.register_set[1].D = sms->cpu.alt.D;
        rts->z80.register_set[1].E = sms->cpu.alt.E;
        rts->z80.register_set[1].F = z80_get_reg_alt(sms, 0x6);
        rts->z80.register_set[1].H = sms->cpu.alt.H;
        rts->z80.register_set[1].L = sms->cpu.alt.L;
        rts->z80.register_set[1].A = sms->cpu.alt.A;

        rts->z80.IFF1 = sms->cpu.IFF1;
        rts->z80.IFF2 = sms->cpu.IFF2;
        rts->z80.ei_delay = sms->cpu.ei_delay;
        rts->z80.execution_mode = sms->cpu.execution_mode;
    }

    /* ---VDP---*/
    {
        memcpy(rts->vdp.registers, sms->vdp.registers, sizeof(rts->vdp.registers));
        memcpy(rts->vdp.cram, sms->vdp.cram, sizeof(rts->vdp.cram));
        memcpy(rts->vdp.vram, sms->vdp.vram, sizeof(rts->vdp.vram));

        for (size_t i = 0; i < ARRAY_SIZE(rts->vdp.sprites); i++)
        {
            rts->vdp.sprites[i].y = sms->vdp.sprites[i].y;
            rts->vdp.sprites[i].x = sms->vdp.sprites[i].x;
            rts->vdp.sprites[i].tile_num = sms->vdp.sprites[i].tile_num;
            rts->vdp.sprites[i].colour = sms->vdp.sprites[i].colour;
        }

        rts->vdp.sprites_count = sms->vdp.sprites_count;
        rts->vdp.addr = sms->vdp.addr;
        rts->vdp.code = sms->vdp.code;
        rts->vdp.cram_gg_latch = sms->vdp.cram_gg_latch;
        rts->vdp.vertical_scroll = sms->vdp.vertical_scroll;
        rts->vdp.hcount = sms->vdp.hcount;
        rts->vdp.vcount = sms->vdp.vcount;
        rts->vdp.line_counter = sms->vdp.line_counter;
        rts->vdp.buffer_read_data = sms->vdp.buffer_read_data;
        rts->vdp.control_word = sms->vdp.control_word;
        rts->vdp.control_latch = sms->vdp.control_latch;
        rts->vdp.frame_interrupt_pending = sms->vdp.frame_interrupt_pending;
        rts->vdp.line_interrupt_pending = sms->vdp.line_interrupt_pending;
        rts->vdp.sprite_overflow = sms->vdp.sprite_overflow;
        rts->vdp.sprite_collision = sms->vdp.sprite_collision;
        rts->vdp.fifth_sprite_num = sms->vdp.fifth_sprite_num;
        rts->vdp.state = sms->vdp.state;
        rts->vdp.nmi_pending = sms->vdp.nmi_pending;
    }

    /* ---CART---*/
    {
        memcpy(rts->cart.ram, sms->cart_ram.ram, sizeof(rts->cart.ram));
        rts->cart.sram_used = sms->cart_ram.used;
        memcpy(rts->cart.gg_regs, sms->port.gg_regs, sizeof(sms->port.gg_regs));
        memcpy(&rts->cart.cart_mapper, &sms->cart.mappers, sizeof(sms->cart.mappers));
        memcpy(&rts->cart.bios_mapper, &sms->cart_bios.mappers, sizeof(sms->cart_bios.mappers));
    }

    /* ---memory_control---*/
    {
        rts->memory_control.exp_slot_disable = sms->memory_control.exp_slot_disable;
        rts->memory_control.cart_slot_disable = sms->memory_control.cart_slot_disable;
        rts->memory_control.card_slot_disable = sms->memory_control.card_slot_disable;
        rts->memory_control.work_ram_disable = sms->memory_control.work_ram_disable;
        rts->memory_control.bios_rom_disable = sms->memory_control.bios_rom_disable;
        rts->memory_control.io_chip_disable = sms->memory_control.io_chip_disable;
    }

    /* ---apu---*/
    {
        assert(psg_state_size(sms->psg, false) < sizeof(rts->psg));
        if (config->include_psg_blip)
        {
            // write to the end of state.
            psg_save_state(sms->psg, (uint8_t*)data + sizeof(struct Rts), state_size, config->include_psg_blip);
        }
        else
        {
            psg_save_state(sms->psg, rts->psg, sizeof(rts->psg), config->include_psg_blip);
        }
    }

    /* ---scheduler---*/
    {
        assert(scheduler_state_size(&sms->scheduler) < sizeof(rts->scheduler));
        scheduler_save_state(&sms->scheduler, rts->scheduler, sizeof(rts->scheduler));
    }

    /* ---system_ram---*/
    {
        memcpy(rts->system_ram, sms->system_ram, sizeof(rts->system_ram));
    }

    return true;
}

bool SMS_loadstate(struct SMS_Core* sms, const void* data, size_t size, const struct SMS_StateConfig* config)
{
    config = state_get_config(config);

    const size_t state_size = SMS_get_state_size(sms, config);
    if (size < state_size)
    {
        return false;
    }

    const struct Rts* rts = data;

    /* verify meta */
    if (rts->meta.magic != STATE_MAGIC)
    {
        SMS_log("bad savestate, invalid magic. got: 0x%04X wanted: 0x%04X\n", rts->meta.magic, STATE_MAGIC);
        return false;
    }

    if (rts->meta.rom_crc32 != sms->crc)
    {
        SMS_log("bad savestate, invalid crc. got: 0x%04X wanted: 0x%04X\n", rts->meta.rom_crc32, sms->crc);
        return false;
    }

    if (rts->meta.state_version_major != STATE_VERSION_MAJOR) {
        SMS_log("bad savestate, bad version. got: %u wanted: %u\n", rts->meta.state_version_major, STATE_VERSION_MAJOR);
        return false;
    }

    if (rts->meta.region != sms->region) {
        SMS_log("bad savestate, bad region. got: %u wanted: %u\n", rts->meta.region, sms->region);
        return false;
    }

    if (rts->meta.console != sms->console) {
        SMS_log("bad savestate, bad console. got: %u wanted: %u\n", rts->meta.console, sms->console);
        return false;
    }

    /* ---Z80---*/
    {
        sms->cpu.PC = rts->z80.PC;
        sms->cpu.SP = rts->z80.SP;
        sms->cpu.IXL = rts->z80.IXL;
        sms->cpu.IXH = rts->z80.IXH;
        sms->cpu.IYL = rts->z80.IYL;
        sms->cpu.IYH = rts->z80.IYH;
        sms->cpu.I = rts->z80.I; // interrupt vector
        sms->cpu.R = rts->z80.R; // memory refresh

        sms->cpu.main.B = rts->z80.register_set[0].B;
        sms->cpu.main.C = rts->z80.register_set[0].C;
        sms->cpu.main.D = rts->z80.register_set[0].D;
        sms->cpu.main.E = rts->z80.register_set[0].E;
        z80_set_reg_main(sms,rts->z80.register_set[0].F, 0x6);
        sms->cpu.main.H = rts->z80.register_set[0].H;
        sms->cpu.main.L = rts->z80.register_set[0].L;
        sms->cpu.main.A = rts->z80.register_set[0].A;

        sms->cpu.alt.B = rts->z80.register_set[1].B;
        sms->cpu.alt.C = rts->z80.register_set[1].C;
        sms->cpu.alt.D = rts->z80.register_set[1].D;
        sms->cpu.alt.E = rts->z80.register_set[1].E;
        z80_set_reg_alt(sms,rts->z80.register_set[1].F, 0x6);
        sms->cpu.alt.H = rts->z80.register_set[1].H;
        sms->cpu.alt.L = rts->z80.register_set[1].L;
        sms->cpu.alt.A = rts->z80.register_set[1].A;

        sms->cpu.IFF1 = rts->z80.IFF1;
        sms->cpu.IFF2 = rts->z80.IFF2;
        sms->cpu.ei_delay = rts->z80.ei_delay;
        sms->cpu.execution_mode = rts->z80.execution_mode;
    }

    /* ---VDP---*/
    {
        memcpy(sms->vdp.registers, rts->vdp.registers, sizeof(rts->vdp.registers));
        memcpy(sms->vdp.cram, rts->vdp.cram, sizeof(rts->vdp.cram));
        memcpy(sms->vdp.vram, rts->vdp.vram, sizeof(rts->vdp.vram));

        for (size_t i = 0; i < ARRAY_SIZE(rts->vdp.sprites); i++)
        {
            sms->vdp.sprites[i].y = rts->vdp.sprites[i].y;
            sms->vdp.sprites[i].x = rts->vdp.sprites[i].x;
            sms->vdp.sprites[i].tile_num = rts->vdp.sprites[i].tile_num;
            sms->vdp.sprites[i].colour = rts->vdp.sprites[i].colour;
        }

        sms->vdp.sprites_count = rts->vdp.sprites_count;
        sms->vdp.addr = rts->vdp.addr;
        sms->vdp.code = rts->vdp.code;
        sms->vdp.cram_gg_latch = rts->vdp.cram_gg_latch;
        sms->vdp.vertical_scroll = rts->vdp.vertical_scroll;
        sms->vdp.hcount = rts->vdp.hcount;
        sms->vdp.vcount = rts->vdp.vcount;
        sms->vdp.line_counter = rts->vdp.line_counter;
        sms->vdp.buffer_read_data = rts->vdp.buffer_read_data;
        sms->vdp.control_word = rts->vdp.control_word;
        sms->vdp.control_latch = rts->vdp.control_latch;
        sms->vdp.frame_interrupt_pending = rts->vdp.frame_interrupt_pending;
        sms->vdp.line_interrupt_pending = rts->vdp.line_interrupt_pending;
        sms->vdp.sprite_overflow = rts->vdp.sprite_overflow;
        sms->vdp.sprite_collision = rts->vdp.sprite_collision;
        sms->vdp.fifth_sprite_num = rts->vdp.fifth_sprite_num;
        sms->vdp.state = rts->vdp.state;
        sms->vdp.nmi_pending = rts->vdp.nmi_pending;
    }

    /* ---CART---*/
    {
        memcpy(sms->cart_ram.ram, rts->cart.ram, sizeof(rts->cart.ram));
        memcpy(&sms->cart.mappers, &rts->cart.cart_mapper, sizeof(sms->cart.mappers));
        memcpy(&sms->cart_bios.mappers, &rts->cart.bios_mapper, sizeof(sms->cart_bios.mappers));
        sms->cart_ram.used = rts->cart.sram_used;

        if (state_version_atleast(rts, 2, 3)) {
            memcpy(sms->port.gg_regs, rts->cart.gg_regs, sizeof(sms->port.gg_regs));
        }
    }

    /* ---memory_control---*/
    {
        sms->memory_control.exp_slot_disable = rts->memory_control.exp_slot_disable;
        sms->memory_control.cart_slot_disable = rts->memory_control.cart_slot_disable;
        sms->memory_control.card_slot_disable = rts->memory_control.card_slot_disable;
        sms->memory_control.work_ram_disable = rts->memory_control.work_ram_disable;
        sms->memory_control.bios_rom_disable = rts->memory_control.bios_rom_disable;
        sms->memory_control.io_chip_disable = rts->memory_control.io_chip_disable;
    }

    /* ---apu---*/
    {
        assert(psg_state_size(sms->psg, false) < sizeof(rts->psg));
        if (config->include_psg_blip)
        {
            // load from the end of state.
            psg_load_state(sms->psg, (const uint8_t*)data + sizeof(struct Rts), state_size, config->include_psg_blip);
        }
        else
        {
            psg_load_state(sms->psg, rts->psg, sizeof(rts->psg), config->include_psg_blip);
        }
    }


    /* ---scheduler---*/
    {
        assert(scheduler_state_size(&sms->scheduler) < sizeof(rts->scheduler));
        scheduler_load_state(&sms->scheduler, rts->scheduler, sizeof(rts->scheduler));

        for (unsigned i = 0; i < SchedulerID_MAX; i++) {
            if (scheduler_has_event(&sms->scheduler, i)) {
                const int cycles = scheduler_get_event_cycles_absolute(&sms->scheduler, i);
                switch (i) {
                    case SchedulerID_VDP: scheduler_add_absolute(&sms->scheduler, i, cycles, vdp_on_event, sms); break;
                    case SchedulerID_IRQ: scheduler_add_absolute(&sms->scheduler, i, cycles, z80_on_irq_event, sms); break;
                    case SchedulerID_HALT: scheduler_add_absolute(&sms->scheduler, i, cycles, z80_on_halt_event, sms); break;
                }
            }
        }

        // override reset event
        const unsigned reset_id = scheduler_get_reset_event_id(&sms->scheduler);
        const int reset_cycles = scheduler_get_event_cycles_absolute(&sms->scheduler, reset_id);
        scheduler_add_absolute(&sms->scheduler, reset_id, reset_cycles, timeout_event, sms);
    }

    /* ---system_ram---*/
    {
        memcpy(sms->system_ram, rts->system_ram, sizeof(rts->system_ram));
    }

    // we need to reload the mapper pointers!
    mapper_update(sms);
    vdp_mark_palette_dirty(sms);

    return true;
}
