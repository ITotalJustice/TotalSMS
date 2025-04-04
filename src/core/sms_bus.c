#include "sms.h"
#include "sms_internal.h"
#include "sms_types.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


// this is mapped to any address space which is unmapped
// and *not* mirrored, such as sega mapper writing to 0x0-0x8000 range
static const uint8_t READ_UNUSED_BANK[0x400];
static uint8_t WRITE_UNUSED_BANK[0x400];

static bool sega_mapper_control_rom_write_enable(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->cart.mappers.sega.fffc, 7);
}

static bool sega_mapper_control_ram_enable_c0000(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->cart.mappers.sega.fffc, 4);
}

static bool sega_mapper_control_ram_enable_80000(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->cart.mappers.sega.fffc, 3);
}

static bool sega_mapper_control_ram_bank_select(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->cart.mappers.sega.fffc, 2);
}

static uint8_t sega_mapper_control_bank_shift(const struct SMS_Core* sms)
{
    return sms->cart.mappers.sega.fffc & 0x3;
}

static void sega_mapper_update_slot0(struct SMS_Core* sms)
{
    // if we have bios and is loaded, do not map rom here!
    if (SMS_has_bios(sms) && !sms->memory_control.bios_rom_disable)
    {
        return;
    }

    const size_t offset = 0x4000 * sms->cart.mappers.sega.fffd;

    // this is fixed, never updated!
    sms->rmap[0x00] = sms->rom;

    for (size_t i = 1; i < 0x10; i++)
    {
        // only the first 15 banks are saved
        sms->rmap[i] = sms->rom + offset + (0x400 * i);
    }
}

static void sega_mapper_update_slot1(struct SMS_Core* sms)
{
    const size_t offset = 0x4000 * sms->cart.mappers.sega.fffe;

    for (size_t i = 0; i < 0x10; i++)
    {
        sms->rmap[i + 0x10] = sms->rom + offset + (0x400 * i);
    }
}

static void sega_mapper_update_slot2(struct SMS_Core* sms)
{
    const size_t offset = 0x4000 * sms->cart.mappers.sega.ffff;

    for (size_t i = 0; i < 0x10; i++)
    {
        sms->rmap[i + 0x20] = sms->rom + offset + (0x400 * i);
        sms->wmap[i + 0x20] = WRITE_UNUSED_BANK;
    }
}

static void sega_mapper_update_ram0(struct SMS_Core* sms)
{
    sms->cart.sram_used = true;
    sms->cart.sram_dirty = true;
    const bool ram_bank_select = sega_mapper_control_ram_bank_select(sms);

    for (size_t i = 0; i < 0x10; i++)
    {
        sms->rmap[i + 0x20] = sms->cart.ram[ram_bank_select] + (0x400 * i);
        sms->wmap[i + 0x20] = sms->cart.ram[ram_bank_select] + (0x400 * i);
    }
}

static void setup_mapper_unused_ram(struct SMS_Core* sms)
{
    for (size_t i = 0; i < ARRAY_SIZE(sms->rmap); i++)
    {
        sms->rmap[i] = READ_UNUSED_BANK;
        sms->wmap[i] = WRITE_UNUSED_BANK;
    }
}

static void setup_mapper_none(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x30; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    // sg only has 1k of ram
    // todo: maybe have different variants of MAPPER_NONE for each system?
    // such as sms, sg, msx etc
    const uint8_t ram_mask = SMS_is_system_type_sg(sms) ? 0x0 : 0x7;

    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram + (0x400 * (i & ram_mask));
        sms->wmap[0x30 + i] = sms->system_ram + (0x400 * (i & ram_mask));
    }
}

static void init_mapper_sega(struct SMS_Core* sms)
{
    // control is reset to zero
    sms->cart.mappers.sega.fffc = 0;
    // default banks
    sms->cart.mappers.sega.fffd = 0;
    sms->cart.mappers.sega.fffe = 1;
    sms->cart.mappers.sega.ffff = 2;
}

static void setup_mapper_sega(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
        sms->wmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
    }

    sega_mapper_update_slot0(sms);
    sega_mapper_update_slot1(sms);
    sega_mapper_update_slot2(sms);
}

static void codemaster_mapper_update_slot(struct SMS_Core* sms, const uint8_t slot, const uint8_t value)
{
    sms->cart.mappers.codemasters.slot[slot] = value % sms->cart.max_bank_mask;
    const size_t offset = 0x4000 * sms->cart.mappers.codemasters.slot[slot];

    for (size_t i = 0; i < 0x10; i++)
    {
        sms->rmap[i + (0x10 * slot)] = sms->rom + offset + (0x400 * i);
    }
}

static void codemaster_mapper_update_slot0(struct SMS_Core* sms, const uint8_t value)
{
    codemaster_mapper_update_slot(sms, 0, value);
}

static void codemaster_mapper_update_slot1(struct SMS_Core* sms, const uint8_t value)
{
    // for codemaster games that feature on-cart ram
    // writing to ctrl-1 with bit7 set maps ram 0xA000-0xC000
    if (IS_BIT_SET(value, 7))
    {
        for (size_t i = 0; i < 0x8; i++)
        {
            sms->rmap[i + 0x28] = sms->cart.ram[0] + (0x400 * i);
            sms->wmap[i + 0x28] = sms->cart.ram[0] + (0x400 * i);
        }

        sms->cart.mappers.codemasters.ram_mapped = true;
        return;
    }
    // was ram previously mapped?
    else if (sms->cart.mappers.codemasters.ram_mapped)
    {
        sms->cart.mappers.codemasters.ram_mapped = false;

        // unmap ram and re-map the rom
        for (size_t i = 0; i < 0x8; i++)
        {
            sms->wmap[i + 0x28] = WRITE_UNUSED_BANK;
        }

        codemaster_mapper_update_slot(sms, 2, sms->cart.mappers.codemasters.slot[2]);
    }

    codemaster_mapper_update_slot(sms, 1, value);
}

static void codemaster_mapper_update_slot2(struct SMS_Core* sms, const uint8_t value)
{
    // NOTE: if ram is mapped (ernie elfs golf), then does writes to this
    // reg remap rom? or are they ignored?
    if (sms->cart.mappers.codemasters.ram_mapped)
    {
        sms->cart.mappers.codemasters.slot[2] = value % sms->cart.max_bank_mask;
        const size_t offset = 0x4000 * sms->cart.mappers.codemasters.slot[2];

        for (size_t i = 0; i < 0x8; i++)
        {
            sms->rmap[i + 0x20] = sms->rom + offset + (0x400 * i);
        }
    }
    else
    {
        codemaster_mapper_update_slot(sms, 2, value);
    }
}

static void init_mapper_codemaster(struct SMS_Core* sms)
{
    sms->cart.mappers.codemasters.slot[0] = 0;
    sms->cart.mappers.codemasters.slot[1] = 1;
    sms->cart.mappers.codemasters.slot[2] = 2;
    sms->cart.mappers.codemasters.ram_mapped = false;
}

static void setup_mapper_codemaster(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
        sms->wmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
    }

    codemaster_mapper_update_slot0(sms, sms->cart.mappers.codemasters.slot[0]);
    codemaster_mapper_update_slot1(sms, sms->cart.mappers.codemasters.slot[1]);
    codemaster_mapper_update_slot2(sms, sms->cart.mappers.codemasters.slot[2]);
}

static void korean_mapper_update_slot2(struct SMS_Core* sms, const uint8_t value)
{
    sms->cart.mappers.korean.slot2 = value % sms->cart.max_bank_mask;
    const size_t offset = 0x4000 * sms->cart.mappers.korean.slot2;

    for (size_t i = 0; i < 0x10; i++)
    {
        sms->rmap[i + 0x20] = sms->rom + offset + (0x400 * i);
    }
}

static void init_mapper_korean(struct SMS_Core* sms)
{
    sms->cart.mappers.korean.slot2 = 2;
}

static void setup_mapper_korean(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x20; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
        sms->wmap[0x30 + i] = sms->system_ram + (0x400 * (i & 0x7));
    }

    korean_mapper_update_slot2(sms, sms->cart.mappers.korean.slot2);
}

static void setup_mapper_dahjee_a(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x30; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    // has 8K mapped at 0x2000-0x3FFF
    for (int i = 0; i < 0x8; i++)
    {
        sms->rmap[0x8 + i] = sms->cart.ram[0] + (0x400 * i);
        sms->wmap[0x8 + i] = sms->cart.ram[0] + (0x400 * i);
    }

    // normal 1K mapping 0xC000-0xFFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram;
        sms->wmap[0x30 + i] = sms->system_ram;
    }
}

static void setup_mapper_dahjee_b(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x30; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    // 8K ram mapped 0xC000-0xFFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->cart.ram[0] + (0x400 * (i&7));
        sms->wmap[0x30 + i] = sms->cart.ram[0] + (0x400 * (i&7));
    }
}

static void setup_mapper_castle(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x20; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    // 8k ram mapping 0x8000-0xBFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x20 + i] = sms->cart.ram[0] + (0x400 * (i&7));
        sms->wmap[0x20 + i] = sms->cart.ram[0] + (0x400 * (i&7));
    }

    // normal 1k mapping 0xC000-0xFFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms->rmap[0x30 + i] = sms->system_ram;
        sms->wmap[0x30 + i] = sms->system_ram;
    }
}

static void setup_mapper_othello(struct SMS_Core* sms)
{
    for (int i = 0; i < 0x20; i++)
    {
        sms->rmap[i] = sms->rom + 0x400 * (i % sms->rom_mask);
    }

    // 2K ram mapped 0x8000-0xFFFF
    for (int i = 0; i < 0x20; i++)
    {
        sms->rmap[0x20 + i] = sms->cart.ram[0] + (0x400 * (i & 1));
        sms->wmap[0x20 + i] = sms->cart.ram[0] + (0x400 * (i & 1));
    }
}

static void sega_mapper_fffx_write(struct SMS_Core* sms, const uint16_t addr, const uint8_t value)
{
    assert(sms->cart.mapper_type == MAPPER_TYPE_SEGA && "wrong mapper, how did we get here?!?");

    switch (addr)
    {
        case 0xFFFC: // Cartridge RAM mapper control
            sms->cart.mappers.sega.fffc = value;
            // TODO: mapping at 0xC000
            // assert(!sms->cart.mappers.sega.fffc.rom_write_enable && "rom write enable!");
            assert(!sega_mapper_control_ram_enable_c0000(sms) && "unimp ram_enable_c0000");
            assert(!sega_mapper_control_bank_shift(sms) && "unimp bank_shift");

            if (sega_mapper_control_ram_enable_80000(sms))
            {
                sega_mapper_update_ram0(sms);
                SMS_log("game is mapping sram to rom region 0x8000!\n");
            }
            else
            {
                // unamp ram
                sega_mapper_update_slot2(sms);
            }
            break;

        case 0xFFFD: // Mapper slot 0 control
            sms->cart.mappers.sega.fffd = value % sms->cart.max_bank_mask;
            sega_mapper_update_slot0(sms);
            break;

        case 0xFFFE: // Mapper slot 1 control
            sms->cart.mappers.sega.fffe = value % sms->cart.max_bank_mask;
            sega_mapper_update_slot1(sms);
            break;

        case 0xFFFF: // Mapper slot 2 control
            sms->cart.mappers.sega.ffff = value % sms->cart.max_bank_mask;
            if (sega_mapper_control_ram_enable_80000(sms) == false)
            {
                sega_mapper_update_slot2(sms);
            }
            break;
    }
}

static void mapper_write(struct SMS_Core* sms, const uint16_t addr, const uint8_t value)
{
    // specific mapper writes
    switch (sms->cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            if UNLIKELY(addr >= 0xFFFC)
            {
                sega_mapper_fffx_write(sms, addr, value);
            }
            break;

        // control regs are not mirrored to ram (written values cannot be read back)
        case MAPPER_TYPE_CODEMASTERS:
            if UNLIKELY(addr <= 0x3FFF)
            {
                assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                codemaster_mapper_update_slot0(sms, value);
                return;
            }
            else if UNLIKELY(addr >= 0x4000 && addr <= 0x7FFF)
            {
                assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                codemaster_mapper_update_slot1(sms, value);
                return;
            }
            else if UNLIKELY(addr >= 0x8000 && addr <= 0xBFFF)
            {
                if (!sms->cart.mappers.codemasters.ram_mapped || addr <= 0x9FFF)
                {
                    assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                    codemaster_mapper_update_slot2(sms, value);
                    return;
                }
            }
            break;

        // control reg is not mirrored to ram (written values cannot be read back)
        case MAPPER_TYPE_KOREAN:
            if UNLIKELY(addr == 0xA000)
            {
                korean_mapper_update_slot2(sms, value);
                return;
            }
            break;

        case MAPPER_TYPE_NONE:
        case MAPPER_TYPE_DAHJEE_A:
        case MAPPER_TYPE_DAHJEE_B:
        case MAPPER_TYPE_THE_CASTLE:
        case MAPPER_TYPE_OTHELLO:
            break;
    }
}

// https://www.smspower.org/Development/Port3E
static void IO_memory_control_write(struct SMS_Core* sms, const uint8_t value)
{
    const struct SMS_MemoryControlRegister old = sms->memory_control;

    sms->memory_control.exp_slot_disable  = IS_BIT_SET(value, 7);
    sms->memory_control.cart_slot_disable = IS_BIT_SET(value, 6);
    sms->memory_control.card_slot_disable = IS_BIT_SET(value, 5);
    sms->memory_control.work_ram_disable  = IS_BIT_SET(value, 4);
    sms->memory_control.bios_rom_disable  = IS_BIT_SET(value, 3);
    sms->memory_control.io_chip_disable   = IS_BIT_SET(value, 2);

    // bios either unmapped itself (likely) or got re-mapped (impossible)
    if (SMS_has_bios(sms) && old.bios_rom_disable != sms->memory_control.bios_rom_disable)
    {
        // assert(!sms->memory_control.bios_rom_disable && "bios got remapped, this is impossible!");
        // SMS_log("bios unmapped\n");
        mapper_update(sms);
    }

    SMS_log("[memory_control]\n");
    SMS_log("\tmemory_control.exp_slot_disable: %u\n", sms->memory_control.exp_slot_disable);
    SMS_log("\tmemory_control.cart_slot_disable: %u\n", sms->memory_control.cart_slot_disable);
    SMS_log("\tmemory_control.card_slot_disable: %u\n", sms->memory_control.card_slot_disable);
    SMS_log("\tmemory_control.work_ram_disable: %u\n", sms->memory_control.work_ram_disable);
    SMS_log("\tmemory_control.bios_rom_disable: %u\n", sms->memory_control.bios_rom_disable);
    SMS_log("\tmemory_control.io_chip_disable: %u\n", sms->memory_control.io_chip_disable);
    // assert(sms->memory_control.work_ram_disable == 0 && "wram disabled!");
}

// todo: PORT 0x3F
static void IO_control_write(struct SMS_Core* sms, const uint8_t value)
{
    /*
    bit 0: controller 1 button 2 direction (1=input 0=output)
    bit 1: controller 1 TH direction (1=input 0=output)
    bit 2: controller 2 button 2 direction (1=input 0=output)
    bit 3: controller 2 TH direction (1=input 0=output)

    bit 4: controller 1 button 2 output level (1=high 0=low)
    bit 5: controller 1 TH output level (1=high 0=low*)
    bit 6: controller 2 button 2 output level (1=high 0=low)
    bit 7: controller 2 TH output level (1=high 0=low*)
    */
    // SMS_log("IO_control_write: 0x%02X\n", value);
    // good enough for region detection
    sms->port.b &= ~(1 << 6);
    sms->port.b &= ~(1 << 7);

    sms->port.b |= ((value >> 5) & 1) << 6;
    sms->port.b |= ((value >> 7) & 1) << 7;
}

static bool gear_to_gear_is_enabled(const struct SMS_Core* sms)
{
    // return true;
    return (sms->port.gg_regs[0x5] & 0x38) == 0x38;
}

// checks if send buffer is full
static bool gear_to_gear_is_write_pending(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->port.gg_regs[0x5], 0);
}

// checks if recieve buffer is full
static bool gear_to_gear_is_read_pending(const struct SMS_Core* sms)
{
    return IS_BIT_SET(sms->port.gg_regs[0x5], 1);
}

// checks if send buffer is full
static void gear_to_gear_set_write_pending(struct SMS_Core* sms, bool is_pending)
{
    sms->port.gg_regs[0x5] &= ~(1 << 0);
    sms->port.gg_regs[0x5] |= is_pending << 0;
}

// checks if recieve buffer is full
static void gear_to_gear_set_read_pending(struct SMS_Core* sms, bool is_pending)
{
    sms->port.gg_regs[0x5] &= ~(1 << 1);
    sms->port.gg_regs[0x5] |= is_pending << 1;
}

#define G2G_BYTE_SENT 0x01
#define G2G_BYTE_RECV 0x02
#define G2G_ERROR 0x04
#define G2G_ENABLE_NMI_ON_RECV 0x08
#define G2G_ENABLE_SEND 0x10
#define G2G_ENABLE_RECV 0x20

static void g2g_baud_rate(const struct SMS_Core* sms)
{
    SMS_log("G2G_BYTE_SENT: %u\n", (sms->port.gg_regs[0x5] & G2G_BYTE_SENT) == G2G_BYTE_SENT);
    SMS_log("G2G_BYTE_RECV: %u\n", (sms->port.gg_regs[0x5] & G2G_BYTE_RECV) == G2G_BYTE_RECV);
    SMS_log("G2G_ERROR: %u\n", (sms->port.gg_regs[0x5] & G2G_ERROR) == G2G_ERROR);
    SMS_log("G2G_ENABLE_NMI_ON_RECV: %u\n", (sms->port.gg_regs[0x5] & G2G_ENABLE_NMI_ON_RECV) == G2G_ENABLE_NMI_ON_RECV);
    SMS_log("G2G_ENABLE_SEND: %u\n", (sms->port.gg_regs[0x5] & G2G_ENABLE_SEND) == G2G_ENABLE_SEND);
    SMS_log("G2G_ENABLE_RECV: %u\n", (sms->port.gg_regs[0x5] & G2G_ENABLE_RECV) == G2G_ENABLE_RECV);
    if ((sms->port.gg_regs[0x5] & 0xF0) == 0xF0)
    {
        SMS_log("G2G_BAUD_300\n");
    }
    if ((sms->port.gg_regs[0x5] & 0xB0) == 0xB0)
    {
        SMS_log("G2G_BAUD_1200\n");
    }
    if ((sms->port.gg_regs[0x5] & 0x70) == 0x70)
    {
        SMS_log("G2G_BAUD_2400\n");
    }
    if ((sms->port.gg_regs[0x5] & 0x30) == 0x30)
    {
        SMS_log("G2G_BAUD_4800\n");
    }
    SMS_log("\n");
}

static uint8_t IO_gamegear_read(struct SMS_Core* sms, const uint8_t addr)
{
    switch (addr & 0x7)
    {
        case 0x0:
            joypad_poll(sms, 1);
            return sms->port.gg_regs[0x0] | 0x1F;
        default: return 0xFF;
        case 0x1:
            SMS_log("[SERIAL-PORT-READ] 0x%02X unk\n", addr);
            return /* 0x7F; */ sms->port.gg_regs[0x1];
        case 0x2:
            SMS_log("[PARALLEL-PORT-READ] 0x%02X direction\n", addr);
            assert(0);
            return /* 0xFF; */ sms->port.gg_regs[0x2];
        case 0x3: return /* 0x00; */ sms->port.gg_regs[0x3];
        case 0x4:
            SMS_log("[SERIAL-PORT-READ] 0x%02X received data\n", addr);
            if (gear_to_gear_is_enabled(sms) && gear_to_gear_is_read_pending(sms))
            {
                gear_to_gear_set_read_pending(sms, false);
                gear_to_gear_set_write_pending(sms, false);
                // assert(0);
                SMS_log("port5 is: 0x%02X\n", sms->port.gg_regs[0x5]);
                // assert(0);
                return sms->port.gg_regs[0x3];
            }
            return /* 0xFF; */ sms->port.gg_regs[0x4];
        case 0x5:
            // SMS_log("[SERIAL-PORT-READ] 0x%02X serial status\n", addr);
            return /* 0x00; */ sms->port.gg_regs[0x5];// | 0x38;
    }

    UNREACHABLE(0xFF);
}

static void IO_gamegear_write(struct SMS_Core* sms, const uint8_t addr, const uint8_t value)
{
    switch (addr & 0x7)
    {
        case 0x1:
            SMS_log("[SERIAL-PORT-WRITE] 0x%02X 0x%02X unk\n", addr, value);
            sms->port.gg_regs[0x1] = value;
            break;
        case 0x2:
            SMS_log("[PARALLEL-PORT-WRITE] 0x%02X 0x%02X direction\n", addr, value);
            sms->port.gg_regs[0x2] = value;
            break;
        case 0x3:
            SMS_log("[SERIAL-PORT-WRITE] 0x%02X 0x%02X send data\n", addr, value);
            if (gear_to_gear_is_enabled(sms) && !gear_to_gear_is_write_pending(sms))
            {
                sms->port.gg_regs[0x3] = value;
                gear_to_gear_set_write_pending(sms, true);
                gear_to_gear_set_read_pending(sms, true);
                SMS_log("port5 is: 0x%02X\n", sms->port.gg_regs[0x5]);
                if (sms->port.gg_regs[0x5] & G2G_ENABLE_NMI_ON_RECV)
                {
                    // z80_nmi(sms);
                }
                // assert(0);
            }
            break;
        case 0x4:
            SMS_log("[SERIAL-PORT-WRITE] 0x%02X 0x%02X received data\n", addr, value);
            assert(0);
            break;
        case 0x5:
            SMS_log("[SERIAL-PORT-WRITE] 0x%02X 0x%02X serial status\n", addr, value);
            sms->port.gg_regs[0x5] = value;
            g2g_baud_rate(sms);
            sms->port.gg_regs[0x5] = 0x30;
            break;

        case 0x6:
            psg_gg_write_io(sms->psg, value, scheduler_get_ticks(&sms->scheduler));
            break;
    }
}

void mapper_update(struct SMS_Core* sms)
{
    // sets all banks to point to unused_bank[0x400]
    setup_mapper_unused_ram(sms);

    switch (sms->cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            setup_mapper_sega(sms);
            break;

        case MAPPER_TYPE_CODEMASTERS:
            setup_mapper_codemaster(sms);
            break;

        case MAPPER_TYPE_KOREAN:
            setup_mapper_korean(sms);
            break;

        case MAPPER_TYPE_NONE:
            setup_mapper_none(sms);
            break;

        case MAPPER_TYPE_DAHJEE_A:
            setup_mapper_dahjee_a(sms);
            break;

        case MAPPER_TYPE_DAHJEE_B:
            setup_mapper_dahjee_b(sms);
            break;

        case MAPPER_TYPE_THE_CASTLE:
            setup_mapper_castle(sms);
            break;

        case MAPPER_TYPE_OTHELLO:
            setup_mapper_othello(sms);
            break;
    }

    // map the bios is enabled (if we have it)
    if (!sms->memory_control.bios_rom_disable)
    {
        assert(SMS_has_bios(sms) && "bios was mapped, but we dont have it!");

        if (SMS_has_bios(sms))
        {
            // usually 8 (8kib)
            const size_t map_max = SMS_MIN(ARRAY_SIZE(sms->rmap), sms->bios_size / 0x400);

            for (size_t i = 0; i < map_max; i++)
            {
                sms->rmap[i] = sms->bios + (0x400 * i);
            }

            // not really needed, but just in case
            for (size_t i = map_max; i < 0x10; i++)
            {
                sms->rmap[i] = sms->rom + 0x400 * i;
            }
        }
    }
}

void mapper_init(struct SMS_Core* sms)
{
    memset(&sms->cart.mappers, 0, sizeof(sms->cart.mappers));
    memset(sms->cart.ram, 0, sizeof(sms->cart.ram));
    sms->cart.sram_used = false;

    switch (sms->cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            init_mapper_sega(sms);
            break;

        case MAPPER_TYPE_CODEMASTERS:
            init_mapper_codemaster(sms);
            break;

        case MAPPER_TYPE_KOREAN:
            init_mapper_korean(sms);
            break;

        case MAPPER_TYPE_NONE:
        case MAPPER_TYPE_DAHJEE_A:
        case MAPPER_TYPE_DAHJEE_B:
        case MAPPER_TYPE_THE_CASTLE:
        case MAPPER_TYPE_OTHELLO:
            break;
    }

    mapper_update(sms);
}

bool mapper_is_sram_mapped(const struct SMS_Core* sms)
{
    return sms->cart.mapper_type == MAPPER_TYPE_SEGA && sega_mapper_control_ram_enable_80000(sms);
}

uint8_t SMS_read8(struct SMS_Core* sms, const uint16_t addr)
{
    assert(sms->rmap[addr >> 10] && "NULL ptr in rmap!");
    return sms->rmap[addr >> 10][addr & 0x3FF];
}

void SMS_write8(struct SMS_Core* sms, const uint16_t addr, const uint8_t value)
{
    mapper_write(sms, addr, value);
    assert(sms->wmap[addr >> 10] && "NULL ptr in wmap!");
    sms->wmap[addr >> 10][addr & 0x3FF] = value;
}

uint16_t SMS_read16(struct SMS_Core* sms, const uint16_t addr)
{
    assert(sms->rmap[addr >> 10] && "NULL ptr in rmap!");
    return mem_read16(sms->rmap[addr >> 10] + (addr & 0x3FF));
}

void SMS_write16(struct SMS_Core* sms, const uint16_t addr, const uint16_t value)
{
    #if SMS_LITTLE_ENDIAN
    mapper_write(sms, addr + 0, value);
    mapper_write(sms, addr + 1, value >> 8);
    memcpy(sms->wmap[addr >> 10] + (addr & 0x3FF), &value, sizeof(value));
    #else
    SMS_write8(sms, addr + 0, value & 0xFF);
    SMS_write8(sms, addr + 1, value >> 8);
    #endif
}

uint8_t SMS_read_io(struct SMS_Core* sms, const uint8_t addr)
{
    switch (addr)
    {
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x04: case 0x05:
            if (SMS_is_system_type_gg(sms))
            {
                return IO_gamegear_read(sms, addr);
            }
            else
            {
                assert(!"reading from gg port in non gg mode, what should happen here?");
                return 0xFF;
            }

        case 0x06: case 0x07:
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
            // note: ristar (GG) reads from port $22 for some reason...
            SMS_log("[PORT-READ] 0x%02X last byte of the instruction\n", addr);
            return 0xFF; // todo:

        case 0x40: case 0x42: case 0x44: case 0x46:
        case 0x48: case 0x4A: case 0x4C: case 0x4E:
        case 0x50: case 0x52: case 0x54: case 0x56:
        case 0x58: case 0x5A: case 0x5C: case 0x5E:
        case 0x60: case 0x62: case 0x64: case 0x66:
        case 0x68: case 0x6A: case 0x6C: case 0x6E:
        case 0x70: case 0x72: case 0x74: case 0x76:
        case 0x78: case 0x7A: case 0x7C: case 0x7E:
        {
            const int a = vdp_io_read_vcounter(sms);
            // assert(a != 0xFF);
            return a;
        }
            return vdp_io_read_vcounter(sms);

        case 0x41: case 0x43: case 0x45: case 0x47:
        case 0x49: case 0x4B: case 0x4D: case 0x4F:
        case 0x51: case 0x53: case 0x55: case 0x57:
        case 0x59: case 0x5B: case 0x5D: case 0x5F:
        case 0x61: case 0x63: case 0x65: case 0x67:
        case 0x69: case 0x6B: case 0x6D: case 0x6F:
        case 0x71: case 0x73: case 0x75: case 0x77:
        case 0x79: case 0x7B: case 0x7D: case 0x7F:
            return vdp_io_read_hcounter(sms);

        case 0x80: case 0x82: case 0x84: case 0x86:
        case 0x88: case 0x8A: case 0x8C: case 0x8E:
        case 0x90: case 0x92: case 0x94: case 0x96:
        case 0x98: case 0x9A: case 0x9C: case 0x9E:
        case 0xA0: case 0xA2: case 0xA4: case 0xA6:
        case 0xA8: case 0xAA: case 0xAC: case 0xAE:
        case 0xB0: case 0xB2: case 0xB4: case 0xB6:
        case 0xB8: case 0xBA: case 0xBC: case 0xBE:
            return vdp_io_data_read(sms);

        case 0x81: case 0x83: case 0x85: case 0x87:
        case 0x89: case 0x8B: case 0x8D: case 0x8F:
        case 0x91: case 0x93: case 0x95: case 0x97:
        case 0x99: case 0x9B: case 0x9D: case 0x9F:
        case 0xA1: case 0xA3: case 0xA5: case 0xA7:
        case 0xA9: case 0xAB: case 0xAD: case 0xAF:
        case 0xB1: case 0xB3: case 0xB5: case 0xB7:
        case 0xB9: case 0xBB: case 0xBD: case 0xBF:
            return vdp_io_status_read(sms);

        case 0xC0: case 0xC2: case 0xC4: case 0xC6:
        case 0xC8: case 0xCA: case 0xCC: case 0xCE:
        case 0xD0: case 0xD2: case 0xD4: case 0xD6:
        case 0xD8: case 0xDA: case 0xDC: case 0xDE:
        case 0xE0: case 0xE2: case 0xE4: case 0xE6:
        case 0xE8: case 0xEA: case 0xEC: case 0xEE:
        case 0xF0: case 0xF2: case 0xF4: case 0xF6:
        case 0xF8: case 0xFA: case 0xFC: case 0xFE:
        case 0xC1: case 0xC3: case 0xC5: case 0xC7:
        case 0xC9: case 0xCB: case 0xCD: case 0xCF:
        case 0xD1: case 0xD3: case 0xD5: case 0xD7:
        case 0xD9: case 0xDB: case 0xDD: case 0xDF:
        case 0xE1: case 0xE3: case 0xE5: case 0xE7:
        case 0xE9: case 0xEB: case 0xED: case 0xEF:
        case 0xF1: case 0xF3: case 0xF5: case 0xF7:
        case 0xF9: case 0xFB: case 0xFD: case 0xFF:
            return joypad_read(sms, addr & 1);
    }

    UNREACHABLE(0xFF);
}

void SMS_write_io(struct SMS_Core* sms, const uint8_t addr, const uint8_t value)
{
    switch (addr)
    {
        // GG regs
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x04: case 0x05: case 0x06:
            if (SMS_is_system_type_gg(sms))
            {
                IO_gamegear_write(sms, addr, value);
            }
            else
            {
                if (addr & 1) // odd / even split
                {
                    IO_control_write(sms, value);
                }
                else
                {
                    IO_memory_control_write(sms, value);
                }
            }
            break;

        case 0x08: case 0x0A: case 0x0C: case 0x0E:
        case 0x10: case 0x12: case 0x14: case 0x16:
        case 0x18: case 0x1A: case 0x1C: case 0x1E:
        case 0x20: case 0x22: case 0x24: case 0x26:
        case 0x28: case 0x2A: case 0x2C: case 0x2E:
        case 0x30: case 0x32: case 0x34: case 0x36:
        case 0x38: case 0x3A: case 0x3C: case 0x3E:
            IO_memory_control_write(sms, value);
            break;

        case 0x07:
        case 0x09: case 0x0B: case 0x0D: case 0x0F:
        case 0x11: case 0x13: case 0x15: case 0x17:
        case 0x19: case 0x1B: case 0x1D: case 0x1F:
        case 0x21: case 0x23: case 0x25: case 0x27:
        case 0x29: case 0x2B: case 0x2D: case 0x2F:
        case 0x31: case 0x33: case 0x35: case 0x37:
        case 0x39: case 0x3B: case 0x3D: case 0x3F:
            IO_control_write(sms, value);
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
            // psg_reg_write(sms, value);
            psg_write_io(sms->psg, value, scheduler_get_ticks(&sms->scheduler));
            break;

        case 0x80: case 0x82: case 0x84: case 0x86:
        case 0x88: case 0x8A: case 0x8C: case 0x8E:
        case 0x90: case 0x92: case 0x94: case 0x96:
        case 0x98: case 0x9A: case 0x9C: case 0x9E:
        case 0xA0: case 0xA2: case 0xA4: case 0xA6:
        case 0xA8: case 0xAA: case 0xAC: case 0xAE:
        case 0xB0: case 0xB2: case 0xB4: case 0xB6:
        case 0xB8: case 0xBA: case 0xBC: case 0xBE:
            vdp_io_data_write(sms, value);
            break;

        case 0x81: case 0x83: case 0x85: case 0x87:
        case 0x89: case 0x8B: case 0x8D: case 0x8F:
        case 0x91: case 0x93: case 0x95: case 0x97:
        case 0x99: case 0x9B: case 0x9D: case 0x9F:
        case 0xA1: case 0xA3: case 0xA5: case 0xA7:
        case 0xA9: case 0xAB: case 0xAD: case 0xAF:
        case 0xB1: case 0xB3: case 0xB5: case 0xB7:
        case 0xB9: case 0xBB: case 0xBD: case 0xBF:
            vdp_io_control_write(sms, value);
            break;
    }
}
