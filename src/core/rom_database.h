#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct RomEntry
{
    uint32_t crc; // crc32
    uint32_t rom; // size
    uint16_t ram; // size
    uint8_t map; // size
    uint8_t sys; // size
};

bool rom_database_find_entry(struct RomEntry* entry, uint32_t crc);

#ifdef __cplusplus
}
#endif
