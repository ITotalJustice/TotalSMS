#include "sms.h"
#include "internal.h"
#include "types.h"
#include "rom_database.h"

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

uint32_t SMS_crc32(uint32_t crc, const void* data, size_t size)
{
    // SOURCE: http://home.thep.lu.se/~bjorn/crc/
    static const uint32_t CRC32_TABLE[0x100] =
    {
        0xD202EF8D, 0xA505DF1B, 0x3C0C8EA1, 0x4B0BBE37, 0xD56F2B94, 0xA2681B02, 0x3B614AB8, 0x4C667A2E, 0xDCD967BF, 0xABDE5729, 0x32D70693, 0x45D03605, 0xDBB4A3A6, 0xACB39330, 0x35BAC28A, 0x42BDF21C,
        0xCFB5FFE9, 0xB8B2CF7F, 0x21BB9EC5, 0x56BCAE53, 0xC8D83BF0, 0xBFDF0B66, 0x26D65ADC, 0x51D16A4A, 0xC16E77DB, 0xB669474D, 0x2F6016F7, 0x58672661, 0xC603B3C2, 0xB1048354, 0x280DD2EE, 0x5F0AE278,
        0xE96CCF45, 0x9E6BFFD3, 0x0762AE69, 0x70659EFF, 0xEE010B5C, 0x99063BCA, 0x000F6A70, 0x77085AE6, 0xE7B74777, 0x90B077E1, 0x09B9265B, 0x7EBE16CD, 0xE0DA836E, 0x97DDB3F8, 0x0ED4E242, 0x79D3D2D4,
        0xF4DBDF21, 0x83DCEFB7, 0x1AD5BE0D, 0x6DD28E9B, 0xF3B61B38, 0x84B12BAE, 0x1DB87A14, 0x6ABF4A82, 0xFA005713, 0x8D076785, 0x140E363F, 0x630906A9, 0xFD6D930A, 0x8A6AA39C, 0x1363F226, 0x6464C2B0,
        0xA4DEAE1D, 0xD3D99E8B, 0x4AD0CF31, 0x3DD7FFA7, 0xA3B36A04, 0xD4B45A92, 0x4DBD0B28, 0x3ABA3BBE, 0xAA05262F, 0xDD0216B9, 0x440B4703, 0x330C7795, 0xAD68E236, 0xDA6FD2A0, 0x4366831A, 0x3461B38C,
        0xB969BE79, 0xCE6E8EEF, 0x5767DF55, 0x2060EFC3, 0xBE047A60, 0xC9034AF6, 0x500A1B4C, 0x270D2BDA, 0xB7B2364B, 0xC0B506DD, 0x59BC5767, 0x2EBB67F1, 0xB0DFF252, 0xC7D8C2C4, 0x5ED1937E, 0x29D6A3E8,
        0x9FB08ED5, 0xE8B7BE43, 0x71BEEFF9, 0x06B9DF6F, 0x98DD4ACC, 0xEFDA7A5A, 0x76D32BE0, 0x01D41B76, 0x916B06E7, 0xE66C3671, 0x7F6567CB, 0x0862575D, 0x9606C2FE, 0xE101F268, 0x7808A3D2, 0x0F0F9344,
        0x82079EB1, 0xF500AE27, 0x6C09FF9D, 0x1B0ECF0B, 0x856A5AA8, 0xF26D6A3E, 0x6B643B84, 0x1C630B12, 0x8CDC1683, 0xFBDB2615, 0x62D277AF, 0x15D54739, 0x8BB1D29A, 0xFCB6E20C, 0x65BFB3B6, 0x12B88320,
        0x3FBA6CAD, 0x48BD5C3B, 0xD1B40D81, 0xA6B33D17, 0x38D7A8B4, 0x4FD09822, 0xD6D9C998, 0xA1DEF90E, 0x3161E49F, 0x4666D409, 0xDF6F85B3, 0xA868B525, 0x360C2086, 0x410B1010, 0xD80241AA, 0xAF05713C,
        0x220D7CC9, 0x550A4C5F, 0xCC031DE5, 0xBB042D73, 0x2560B8D0, 0x52678846, 0xCB6ED9FC, 0xBC69E96A, 0x2CD6F4FB, 0x5BD1C46D, 0xC2D895D7, 0xB5DFA541, 0x2BBB30E2, 0x5CBC0074, 0xC5B551CE, 0xB2B26158,
        0x04D44C65, 0x73D37CF3, 0xEADA2D49, 0x9DDD1DDF, 0x03B9887C, 0x74BEB8EA, 0xEDB7E950, 0x9AB0D9C6, 0x0A0FC457, 0x7D08F4C1, 0xE401A57B, 0x930695ED, 0x0D62004E, 0x7A6530D8, 0xE36C6162, 0x946B51F4,
        0x19635C01, 0x6E646C97, 0xF76D3D2D, 0x806A0DBB, 0x1E0E9818, 0x6909A88E, 0xF000F934, 0x8707C9A2, 0x17B8D433, 0x60BFE4A5, 0xF9B6B51F, 0x8EB18589, 0x10D5102A, 0x67D220BC, 0xFEDB7106, 0x89DC4190,
        0x49662D3D, 0x3E611DAB, 0xA7684C11, 0xD06F7C87, 0x4E0BE924, 0x390CD9B2, 0xA0058808, 0xD702B89E, 0x47BDA50F, 0x30BA9599, 0xA9B3C423, 0xDEB4F4B5, 0x40D06116, 0x37D75180, 0xAEDE003A, 0xD9D930AC,
        0x54D13D59, 0x23D60DCF, 0xBADF5C75, 0xCDD86CE3, 0x53BCF940, 0x24BBC9D6, 0xBDB2986C, 0xCAB5A8FA, 0x5A0AB56B, 0x2D0D85FD, 0xB404D447, 0xC303E4D1, 0x5D677172, 0x2A6041E4, 0xB369105E, 0xC46E20C8,
        0x72080DF5, 0x050F3D63, 0x9C066CD9, 0xEB015C4F, 0x7565C9EC, 0x0262F97A, 0x9B6BA8C0, 0xEC6C9856, 0x7CD385C7, 0x0BD4B551, 0x92DDE4EB, 0xE5DAD47D, 0x7BBE41DE, 0x0CB97148, 0x95B020F2, 0xE2B71064,
        0x6FBF1D91, 0x18B82D07, 0x81B17CBD, 0xF6B64C2B, 0x68D2D988, 0x1FD5E91E, 0x86DCB8A4, 0xF1DB8832, 0x616495A3, 0x1663A535, 0x8F6AF48F, 0xF86DC419, 0x660951BA, 0x110E612C, 0x88073096, 0xFF000000,
    };

    const uint8_t* u8_data = (const uint8_t*)data;

    for (size_t i = 0; i < size; ++i)
    {
        crc = CRC32_TABLE[(uint8_t)crc ^ u8_data[i]] ^ (crc >> 8);
    }

    return crc;
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

bool SMS_is_spiderman_int_hack_enabled(const struct SMS_Core* sms)
{
    return sms->crc == 0xEBE45388;
}

struct SMS_RomHeader SMS_parse_rom_header(const uint8_t* data, uint16_t offset)
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

    memset(sms, 0, sizeof(struct SMS_Core));

    return true;
}

static void SMS_reset(struct SMS_Core* sms)
{
    // do NOT reset cart!
    memset(sms->rmap, 0, sizeof(sms->rmap));
    memset(sms->wmap, 0, sizeof(sms->wmap));
    memset(&sms->cpu, 0, sizeof(sms->cpu));
    memset(&sms->vdp, 0, sizeof(sms->vdp));
    memset(&sms->psg, 0, sizeof(sms->psg));
    memset(&sms->port, 0, sizeof(sms->port));
    memset(sms->system_ram, 0, sizeof(sms->system_ram));

    z80_init(sms);
    psg_init(sms);
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
    sms->cart.max_bank_mask = (size / 0x4000) - 1;
    sms->crc = SMS_crc32(0, rom, size);

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
    sms->cart.max_bank_mask = (size / 0x4000) - 1;
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
    const uint32_t crc = SMS_crc32(0, rom, size);
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
    sms->cart.max_bank_mask = (size / 0x4000) - 1;
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

void SMS_set_pixels(struct SMS_Core* sms, void* pixels, uint16_t pitch, uint8_t bpp)
{
    sms->pixels = pixels;
    sms->pitch = pitch;
    sms->bpp = bpp;
}

void SMS_set_userdata(struct SMS_Core* sms, void* userdata)
{
    sms->userdata = userdata;
}

void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, uint32_t freq)
{
    // avoid div by 0
    if (cb && freq)
    {
        sms->apu_callback = cb;
        sms->apu_callback_freq = (SMS_CPU_CLOCK / freq);
    }
    else
    {
        sms->apu_callback = NULL;
        sms->apu_callback_freq = 0;
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

void SMS_set_better_drums(struct SMS_Core* sms, bool enable)
{
    sms->better_drums = enable;
}

enum { STATE_MAGIC = 0x5E6A };
enum { STATE_VERSION = 2 };

// for savestates, we don't save the port
bool SMS_savestate(const struct SMS_Core* sms, struct SMS_State* state)
{
    state->header.magic = STATE_MAGIC;
    state->header.version = STATE_VERSION;
    state->header.crc = sms->crc;

    memcpy(&state->cpu, &sms->cpu, sizeof(sms->cpu));
    memcpy(&state->vdp, &sms->vdp, sizeof(sms->vdp));
    memcpy(&state->psg, &sms->psg, sizeof(sms->psg));
    memcpy(&state->cart, &sms->cart, sizeof(sms->cart));
    memcpy(&state->memory_control, &sms->memory_control, sizeof(sms->memory_control));
    memcpy(state->system_ram, sms->system_ram, sizeof(sms->system_ram));

    return true;
}

bool SMS_loadstate(struct SMS_Core* sms, const struct SMS_State* state)
{
    if (state->header.magic != STATE_MAGIC)
    {
        SMS_log("bad savestate, invalid magic. got: 0x%04X wanted: 0x%04X\n", state->header.magic, STATE_MAGIC);
        return false;
    }

    if (state->header.version != STATE_VERSION)
    {
        SMS_log("bad savestate, invalid version. got: 0x%04X wanted: 0x%04X\n", state->header.version, STATE_VERSION);
        return false;
    }

    if (state->header.crc != sms->crc)
    {
        SMS_log("bad savestate, invalid crc. got: 0x%04X wanted: 0x%04X\n", state->header.crc, sms->crc);
        return false;
    }

    memcpy(&sms->cpu, &state->cpu, sizeof(sms->cpu));
    memcpy(&sms->vdp, &state->vdp, sizeof(sms->vdp));
    memcpy(&sms->psg, &state->psg, sizeof(sms->psg));
    memcpy(&sms->cart, &state->cart, sizeof(sms->cart));
    memcpy(&sms->memory_control, &state->memory_control, sizeof(sms->memory_control));
    memcpy(sms->system_ram, state->system_ram, sizeof(sms->system_ram));

    // we need to reload the mapper pointers!
    mapper_update(sms);
    vdp_mark_palette_dirty(sms);

    return true;
}

bool SMS_parity16(uint16_t value)
{
    #if HAS_BUILTIN(__builtin_parity) && !defined(N64)
        return !__builtin_parity(value);
    #else
        // SOURCE: https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
        value ^= value >> 8; // 16-bit
        value ^= value >> 4; // 8-bit
        value &= 0xF;
        return !((0x6996 >> value) & 0x1);
    #endif
}

bool SMS_parity8(uint8_t value)
{
    #if HAS_BUILTIN(__builtin_parity) && !defined(N64)
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
    for (size_t i = 0; i < cycles; i += sms->cpu.cycles)
    {
        z80_run(sms);
        vdp_run(sms, sms->cpu.cycles);
        psg_run(sms, sms->cpu.cycles);

        assert(sms->cpu.cycles != 0);
    }

    psg_sync(sms);
}
