#include "sms.h"
#include "scheduler.h"
#include "sms_internal.h"
#include "sms_types.h"
#include "sms_rom_database.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


// not all values are listed here because the other
// values are not used by official software and
// the checksum is broken on those sizes.
static const bool valid_rom_size_values[0x10] =
{
    [0xC] = true, // 32KiB
    [0xE] = true, // 64KiB
    [0xF] = true, // 128KiB
    [0x0] = true, // 256KiB
    [0x1] = true, // 512KiB
};

static const char* const valid_rom_size_string[0x10] =
{
    [0xC] = "32KiB",
    [0xE] = "64KiB",
    [0xF] = "128KiB",
    [0x0] = "256KiB",
    [0x1] = "512KiB",
};

static const char* const region_code_string[0x10] =
{
    [0x3] = "SMS Japan",
    [0x4] = "SMS Export",
    [0x5] = "GG Japan",
    [0x6] = "GG Export",
    [0x7] = "GG International",
};

static void frame_event(void* user, unsigned id, unsigned late)
{
    UNUSED(id);
    UNUSED(late);
    struct SMS_Core* sms = user;
    sms->frame_end = true;
}

void timeout_event(void* user, unsigned id, unsigned late)
{
    struct SMS_Core* sms = user;

    // adust anything that uses timestamps
    psg_update_timestamp(sms->psg, -SCHEDULER_TIMEOUT_CYCLES);
    scheduler_reset_event(&sms->scheduler);
    scheduler_add_absolute(&sms->scheduler, id, SCHEDULER_TIMEOUT_CYCLES, timeout_event, user);
}

static uint16_t find_rom_header_offset(const uint8_t* data)
{
    // loop until we find the magic num
    // the rom header can start at 1 of 3 offsets
    const uint16_t offsets[] =
    {
        // the bios checks in reverse order
        0x7FF0,
        0x3FF0,
        0x1FF0,
    };

    for (size_t i = 0; i < ARRAY_SIZE(offsets); ++i)
    {
        const uint8_t* d = data + offsets[i];
        const char* magic = "TMR SEGA";

        if (d[0] == magic[0] && d[1] == magic[1] &&
            d[2] == magic[2] && d[3] == magic[3] &&
            d[4] == magic[4] && d[5] == magic[5] &&
            d[6] == magic[6] && d[7] == magic[7])
        {
            return offsets[i];
        }
    }

    // invalid offset, this zero needs to be checked by the caller!
    return 0;
}

/* SOURCE: https://web.archive.org/web/20190108202303/http://www.hackersdelight.org/hdcodetxt/crc.c.txt */
uint32_t SMS_crc32(const void* data, size_t size)
{
    #if 0
    int crc;
    unsigned byte, c;
    const unsigned g0 = 0xEDB88320,    g1 = g0>>1,
        g2 = g0>>2, g3 = g0>>3, g4 = g0>>4, g5 = g0>>5,
        g6 = (g0>>6)^g0, g7 = ((g0>>6)^g0)>>1;

    crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++) {
        byte = ((const uint8_t*)data)[i];
        crc = crc ^ byte;
        c = ((crc<<31>>31) & g7) ^ ((crc<<30>>31) & g6) ^
            ((crc<<29>>31) & g5) ^ ((crc<<28>>31) & g4) ^
            ((crc<<27>>31) & g3) ^ ((crc<<26>>31) & g2) ^
            ((crc<<25>>31) & g1) ^ ((crc<<24>>31) & g0);
        crc = ((unsigned)crc >> 8) ^ c;
    }
    return ~crc;
    #else
    int j;
    unsigned int i, byte, crc, mask;
    const unsigned char* message = data;

    i = 0;
    crc = 0xFFFFFFFF;
    while (i < size) {
        byte = message[i];            // Get next byte.
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
    #endif
}

bool SMS_get_skip_audio(const struct SMS_Core* sms)
{
    return sms->skip_audio;
}

bool SMS_get_skip_frame(const struct SMS_Core* sms)
{
    return sms->skip_frame;
}

void SMS_skip_audio(struct SMS_Core* sms, bool enable)
{
    sms->skip_audio = enable;
}

void SMS_skip_frame(struct SMS_Core* sms, bool enable)
{
    sms->skip_frame = enable;
}

void SMS_set_system_type(struct SMS_Core* sms, enum SMS_System system)
{
    sms->system = system;
}

enum SMS_System SMS_get_system_type(const struct SMS_Core* sms)
{
    return sms->system;
}

bool SMS_is_system_type_sms(const struct SMS_Core* sms)
{
    return SMS_get_system_type(sms) == SMS_System_SMS;
}

bool SMS_is_system_type_gg(const struct SMS_Core* sms)
{
    return SMS_get_system_type(sms) == SMS_System_GG;
}

bool SMS_is_system_type_sg(const struct SMS_Core* sms)
{
    return SMS_get_system_type(sms) == SMS_System_SG1000;
}

static struct SMS_RomHeader SMS_parse_rom_header(const uint8_t* data, uint16_t offset)
{
    struct SMS_RomHeader header = {0};

    memcpy(&header.magic, data + offset, sizeof(header.magic));
    // skip 2 padding bytes as well
    offset += sizeof(header.magic) + 2;

    memcpy(&header.checksum, data + offset, sizeof(header.checksum));
    offset += sizeof(header.checksum);

    // the next part depends on if the host is LE or BE.
    // due to needing to read half nibble.
    // for now, assume LE, as it likely will be...
    uint32_t last_4;
    memcpy(&last_4, data + offset, sizeof(last_4));

    #if SMS_LITTLE_ENDIAN
        header.prod_code = 0; // this isn't correct atm
        header.version = (last_4 >> 16) & 0xF;
        header.rom_size = (last_4 >> 24) & 0xF;
        header.region_code = (last_4 >> 28) & 0xF;
    #else
        header.rom_size = last_4 & 0xF;
        // todo: the rest
    #endif

    return header;
}

static void log_header(const struct SMS_RomHeader* header)
{
    // silence warnings if logging is disabled
    UNUSED(header); UNUSED(region_code_string); UNUSED(valid_rom_size_string);

    SMS_log("version: [0x%X]\n", header->version);
    SMS_log("region_code: [0x%X] [%s]\n", header->region_code, region_code_string[header->region_code]);
    SMS_log("rom_size: [0x%X] [%s]\n", header->rom_size, valid_rom_size_string[header->rom_size]);
}

bool SMS_init(struct SMS_Core* sms)
{
    if (!sms)
    {
        return false;
    }

    memset(sms, 0, sizeof(*sms));
    SMS_set_mode1_max_sprites(sms, SMS_MODE1_MAX_SPRITES);
    SMS_set_mode4_max_sprites(sms, SMS_MODE4_MAX_SPRITES);

    if (scheduler_init(&sms->scheduler, SchedulerID_MAX))
    {
        return false;
    }

    if (!(sms->psg = psg_init(SMS_CPU_CLOCK, 44100)))

    for (int i = 0; i < 4; i++)
    {
        sms->volume[i] = 1.0;
    }

    sms->master_volume = 0.5;

    return true;
}

void SMS_quit(struct SMS_Core* sms)
{
    if (sms)
    {
        psg_quit(sms->psg);
        scheduler_quit(&sms->scheduler);
        memset(sms, 0, sizeof(*sms));
    }
}

static void SMS_reset(struct SMS_Core* sms)
{
    // do NOT reset cart!
    memset(sms->rmap, 0, sizeof(sms->rmap));
    memset(sms->wmap, 0, sizeof(sms->wmap));
    memset(&sms->cpu, 0, sizeof(sms->cpu));
    memset(&sms->vdp, 0, sizeof(sms->vdp));
    // memset(&sms->psg, 0, sizeof(sms->psg));
    memset(&sms->port, 0, sizeof(sms->port));
    memset(sms->system_ram, 0, sizeof(sms->system_ram));

    // setup scheduler
    scheduler_reset(&sms->scheduler, 0, timeout_event, sms);

    z80_init(sms);
    if (SMS_is_system_type_sg(sms))
    {
        psg_reset(sms->psg, Sn76489LfsrTappedBit_SG, Sn76489LfsrFeedBit_SG);
    }
    else
    {
        psg_reset(sms->psg, Sn76489LfsrTappedBit_SMS, Sn76489LfsrFeedBit_SMS);
    }
    psg_set_master_volume(sms->psg, 0.5);
    vdp_init(sms);

    // enable everything in control
    memset(&sms->memory_control, 0, sizeof(sms->memory_control));
    // not sure if this needs to be reset...
    memset(&sms->system_ram, 0xFF, sizeof(sms->system_ram));

    // if we don't have bios, disable it in control
    if (!SMS_has_bios(sms))
    {
        sms->memory_control.bios_rom_disable = true;
    }

    // port A/B are hi when a button is NOT pressed
    sms->port.a = 0xFF;
    sms->port.b = 0xFF;

    if (SMS_is_system_type_gg(sms))
    {
        sms->port.gg_regs[0x0] = 0xC0;
        sms->port.gg_regs[0x0] = 0x40;
        sms->port.gg_regs[0x1] = 0x7F;
        sms->port.gg_regs[0x2] = 0xFF;
        sms->port.gg_regs[0x3] = 0x00;
        sms->port.gg_regs[0x4] = 0xFF;
        sms->port.gg_regs[0x5] = 0x00;
        sms->port.gg_regs[0x6] = 0xFF;
    }
}

bool SMS_has_bios(const struct SMS_Core* sms)
{
    // bios should be at least 1-page size in size
    return sms->bios && sms->bios_size >= 1024 && sms->bios_size <= 1024*32;
}

bool SMS_loadbios(struct SMS_Core* sms, const uint8_t* bios, size_t size)
{
    sms->bios = bios;
    sms->bios_size = size;

    // todo: hash all known bios to know exactly what bios is being loaded
    return SMS_has_bios(sms);
}

static bool sg_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size, int system_hint)
{
    assert(system_hint == SMS_System_SG1000);

    SMS_log("[INFO] trying to load sg rom\n");

    // save the rom, setup the size and mask
    sms->rom = rom;
    sms->rom_size = size;
    sms->rom_mask = size / 0x400; // this works because size is always pow2
    sms->cart.max_bank_mask = size / 0x4000;
    sms->crc = SMS_crc32(rom, size);

    SMS_log("crc32 0x%08X\n", sms->crc);

    SMS_set_system_type(sms, system_hint);
    sms->cart.mapper_type = MAPPER_TYPE_NONE;
    SMS_reset(sms);

    // this assumes the game is always sega mapper
    // which (for testing at least), it always will be
    mapper_init(sms);

    return true;
}

static bool loadrom2(struct SMS_Core* sms, struct RomEntry* entry, const uint8_t* rom, size_t size)
{
    // save the rom, setup the size and mask
    sms->rom = rom;
    sms->rom_size = size;
    sms->rom_mask = size / 0x400; // this works because size is always pow2
    sms->cart.max_bank_mask = size / 0x4000;
    sms->crc = entry->crc;

    SMS_set_system_type(sms, entry->sys);
    SMS_reset(sms);

    // this assumes the game is always sega mapper
    // which (for testing at least), it always will be
    sms->cart.mapper_type = entry->map;
    mapper_init(sms);

    return true;
}

bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size, int system_hint)
{
    assert(sms);
    assert(rom);
    assert(size);
    assert(sms && rom && size);

    SMS_log("[INFO] loadrom called with rom size: 0x%zX\n", size);

    struct RomEntry entry = {0};
    const uint32_t crc = SMS_crc32(rom, size);
    SMS_log("crc32 0x%08X\n", crc);

    if (rom_database_find_entry(&entry, crc))
    {
        return loadrom2(sms, &entry, rom, size);
    }
    else
    {
        SMS_log("couldn't find rom in database, checking system hint\n");

        if (system_hint == SMS_System_SG1000)
        {
            SMS_log("system hint is SG1000, trying to load...\n");
            return sg_loadrom(sms, rom, size, system_hint);
        }
        else
        {

        }
    }

    // try to find the header offset
    const uint16_t header_offset = find_rom_header_offset(rom);

    // no header found!
    if (header_offset == 0)
    {
        SMS_log_fatal("[ERROR] unable to find rom header!\n");
        return false;
    }

    SMS_log("[INFO] found header offset at: 0x%X\n", header_offset);

    struct SMS_RomHeader header = SMS_parse_rom_header(rom, header_offset);
    log_header(&header);

    // check if the size is valid
    if (!valid_rom_size_values[header.rom_size])
    {
        SMS_log_fatal("[ERROR] invalid rom size in header! 0x%X\n", header.rom_size);
        return false;
    }

    // save the rom, setup the size and mask
    sms->rom = rom;
    sms->rom_size = size;
    sms->rom_mask = size / 0x400; // this works because size is always pow2
    sms->cart.max_bank_mask = size / 0x4000;
    sms->crc = crc;

    SMS_log("crc32 0x%08X\n", sms->crc);

    if (system_hint != -1)
    {
        SMS_set_system_type(sms, system_hint);
    }
    else if (header.region_code == 0x5 || header.region_code == 0x6 || header.region_code == 0x7)
    {
        SMS_set_system_type(sms, SMS_System_GG);
    }
    else
    {
        SMS_set_system_type(sms, SMS_System_SMS);
    }

    SMS_reset(sms);

    // this assumes the game is always sega mapper
    // which (for testing at least), it always will be
    sms->cart.mapper_type = MAPPER_TYPE_SEGA;
    mapper_init(sms);

    return true;
}

bool SMS_loadsave(struct SMS_Core* sms, const uint8_t* data, size_t size)
{
    if (!data || !size || size != sizeof(sms->cart.ram))
    {
        return false;
    }

    memcpy(sms->cart.ram, data, size);
    return true;
}

bool SMS_used_sram(const struct SMS_Core* sms)
{
    return sms->cart.sram_used;
}

void SMS_set_mode1_max_sprites(struct SMS_Core* sms, uint8_t value)
{
    if (value > ARRAY_SIZE(sms->vdp.sprites))
    {
        sms->mode1_max_spirtes = ARRAY_SIZE(sms->vdp.sprites);
    } else
    {
        sms->mode1_max_spirtes = value;
    }
}

void SMS_set_mode4_max_sprites(struct SMS_Core* sms, uint8_t value)
{
    if (value > ARRAY_SIZE(sms->vdp.sprites))
    {
        sms->mode4_max_spirtes = ARRAY_SIZE(sms->vdp.sprites);
    } else
    {
        sms->mode4_max_spirtes = value;
    }
}

void SMS_set_pixels(struct SMS_Core* sms, void* pixels, uint16_t stride, uint8_t bpp)
{
    sms->pixels = pixels;
    sms->stride = stride;
    sms->bpp = bpp;
}

void SMS_set_builtin_palette(struct SMS_Core* sms, const uint32_t palette[16])
{
    memcpy(sms->builtin_palette, palette, sizeof(sms->builtin_palette));
}

void SMS_set_userdata(struct SMS_Core* sms, void* userdata)
{
    sms->userdata = userdata;
}

void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, uint32_t freq)
{
    if (cb && freq)
    {
        psg_quit(sms->psg);
        sms->psg = psg_init(SMS_CPU_CLOCK, freq);

        if (SMS_is_system_type_sg(sms))
        {
            psg_reset(sms->psg, Sn76489LfsrTappedBit_SG, Sn76489LfsrFeedBit_SG);
        }
        else
        {
            psg_reset(sms->psg, Sn76489LfsrTappedBit_SMS, Sn76489LfsrFeedBit_SMS);
        }

        sms->apu_callback = cb;
    }
    else
    {
        sms->apu_callback = NULL;
    }
}

void SMS_set_vblank_callback(struct SMS_Core* sms, sms_vblank_callback_t cb)
{
    sms->vblank_callback = cb;
}

void SMS_set_colour_callback(struct SMS_Core* sms, sms_colour_callback_t cb)
{
    sms->colour_callback = cb;
}

void SMS_set_input_callback(struct SMS_Core* sms, sms_input_callback_t cb)
{
    sms->input_callback = cb;
}

bool SMS_parity8(uint8_t value)
{
    #if HAS_BUILTIN(__builtin_parity)
        return !__builtin_parity(value);
    #else
        // SOURCE: https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
        value ^= value >> 4; // 8-bit
        value &= 0xF;
        return !((0x6996 >> value) & 0x1);
    #endif
}

void SMS_run(struct SMS_Core* sms, size_t cycles)
{
    sms->frame_end = false;
    scheduler_add(&sms->scheduler, SchedulerID_FRAME, cycles, frame_event, sms);

    if (sms->cpu.execution_mode == Z80_ExecutionMode_HALT)
    {
        z80_halt_loop(sms);
    }

    if (!sms->frame_end)
    {
        for (;;)
        {
            z80_run(sms);
            // scheduler_tick(&sms->scheduler, sms->cpu.cycles);
            if (scheduler_should_fire(&sms->scheduler))
            {
                scheduler_fire(&sms->scheduler);
                if (sms->frame_end)
                {
                    break;
                }

                if (sms->cpu.execution_mode == Z80_ExecutionMode_HALT)
                {
                    z80_halt_loop(sms);
                }
            }
        }
    }
}
