#include "sms.h"
#include "internal.h"

#include <assert.h>
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

static const char* valid_rom_size_string[0x10] =
{
    [0xC] = "32KiB",
    [0xE] = "64KiB",
    [0xF] = "128KiB",
    [0x0] = "256KiB",
    [0x1] = "512KiB",
};

static const char* region_code_string[0x10] =
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
    static const uint16_t offsets[] =
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

    header.prod_code = 0; // this isn't correct atm
    header.version = (last_4 >> 16) & 0xF;
    header.region_code = (last_4 >> 28) & 0xF;
    header.rom_size = (last_4 >> 24) & 0xF;

    return header;
}

static void log_header(const struct SMS_RomHeader* header)
{
    (void)header;

    SMS_log("version: [0x%X]\n", header->version);
    SMS_log("region_code: [0x%X] [%s]\n", header->region_code, region_code_string[header->region_code]);
    SMS_log("rom_size: [0x%X] [%s]\n", header->rom_size, valid_rom_size_string[header->rom_size]);
}

static void setup_mapper(struct SMS_Core* sms)
{
    // this is where it would figure out which mapper
    // to use, for now, hardcode sega mapper (99% of games)
    sms->cart.mapper_type = MAPPER_TYPE_SEGA;
    sega_mapper_setup(sms);
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
    SN76489_init(sms);

    // setup cpu regs
    sms->cpu.PC = 0x0000; // i think?
    sms->cpu.SP = 0xFFFF; // i think?
    sms->cpu.main.A = 0xFF;

    // port A/B are hi when a button is NOT pressed
    sms->port.a = 0xFF;
    sms->port.b = 0xFF;
}

bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size)
{
    assert(sms && rom && size);

    SMS_log("[INFO] loadrom called with rom size: 0x%lX\n", size);

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
    memcpy(sms->cart.rom, rom, size);
    sms->cart.rom_size = size;
    sms->cart.rom_mask = size - 1; // this works because size is always pow2
    
    // this assumes the game is always sega mapper
    // which (for testing at least), it always will be
    setup_mapper(sms);

    SMS_reset(sms);

    return true;
}

void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, void* user, uint32_t freq)
{
    sms->apu.callback = cb;
    sms->apu.callback_user = user;
    sms->apu.freq = freq;
}

bool SMS_parity(unsigned value)
{
    #if HAS_BUILTIN(__builtin_parity)
        return !__builtin_parity(value);
    #else
        // SOURCE: https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
        // value ^= value >> 16; // 32-bit
        value ^= value >> 8; // 16-bit
        value ^= value >> 4; // 8-bit
        value &= 0xF;
        return (0x6996 >> value) & 0x1;
    #endif
}

void SMS_step(struct SMS_Core* sms)
{
    Z80_run(sms);
    // timings are very messed up atm so this is a hack for games to
    // sound nice for now at least
    vdp_run(sms, sms->cpu.cycles * 1.6);
    SN76489_run(sms, sms->cpu.cycles);

    assert(sms->cpu.cycles != 0);
}

void SMS_run_frame(struct SMS_Core* sms)
{
    for (size_t i = 0; i < CYCLES_PER_FRAME; i += sms->cpu.cycles)
    {
        SMS_step(sms);
    }
}
