#include "gamegenie.h"

#include <stdlib.h>
#include <string.h>


bool gamegenie_init(struct GameGenie* gg)
{
    if (!gg)
    {
        return false;
    }

    memset(gg, 0, sizeof(struct GameGenie));
    return true;
}

void gamegenie_exit(struct GameGenie* gg)
{
    gamegenie_reset(gg);
}

static uint8_t hex(char c)
{
    if ((c >= 'A' && c <= 'F'))
    {
        return c - 'A' + 10;
    }

    if ((c >= 'a' && c <= 'f'))
    {
        return c - 'a' + 10;
    }

    if ((c >= '0' && c <= '9'))
    {
        return c - '0';
    }

    return 0xFF;
}

gamegenie_id_t gamegenie_add_cheat(struct GameGenie* gg, const char* s)
{
    // we take into account the '-' in the cheat
    // since all cheats online, and in the cheat books
    // use this format.
    const size_t len = strlen(s);

    uint8_t type = 0;

    if (len == GameGenieType_1)
    {
        type = GameGenieType_1;
    }
    else if (len == GameGenieType_2)
    {
        type = GameGenieType_2;
    }
    else
    {
        return GAMEGENIE_INVALID_ID;
    }

    char hexv[GameGenieType_1];

    for (size_t i = 0; i < len; ++i)
    {
        hexv[i] = hex(s[i]);

        // check that all the hex values were valid.
        // the hex func returns a nibble (4-bits) at a time
        // so any number greater than 0xF is invalid.
        // don't return GAMEGENIE_INVALID_ID if '-' was the error!
        if (hexv[i] > 0xF && ((i + 1) % 3) == 0)
        {
            return GAMEGENIE_INVALID_ID;
        }
    }

    gamegenie_id_t id = 0;

    for (size_t i = 0; i < sizeof(gamegenie_id_t) * 2; ++i)
    {
        id |= hexv[i] << (i * 4);
    }

    const uint8_t replace_value = hexv[0] << 4 | hexv[1];

    // MSB nibble of addr is complemented
    const uint16_t addr = ((hexv[6] ^ 0xF) << 12) | hexv[2] << 8 | hexv[4] << 4 | hexv[5];

    uint8_t compare_value = 0;

    // type1 has the compare value
    if (type == GameGenieType_1)
    {
        compare_value = hexv[8] << 4 | hexv[10];
        // invert
        compare_value ^= 0xFF;
        // rotate by 2
        compare_value = (compare_value >> 2) | ((compare_value & 0x3) << 6);
        // invert bits 0,2,6
        compare_value ^= 0x45;
    }

    struct GameGenieEntry entry = {0};
    entry.addr = addr;
    entry.new_value = replace_value;
    entry.old_value = 0;
    entry.compare = compare_value;
    entry.id = id;
    entry.type = type;
    entry.set = false;

    struct GameGenieEntry* new_entry = (struct GameGenieEntry*)malloc(sizeof(struct GameGenieEntry));
    *new_entry = (struct GameGenieEntry){
        .next = NULL,
        .id = id,
        .type = type,
        .addr = addr,
        .new_value = replace_value,
        .old_value = 0,
        .compare = compare_value,
        .set = false,
    };

    if (gg->entries == NULL)
    {
        gg->entries = new_entry;
    }
    else
    {
        struct GameGenieEntry* _entry = gg->entries;

        while (_entry->next)
        {
            _entry = _entry->next;
        }

        _entry->next = new_entry;
    }

    gg->count++;

    return id;
}

void gamegenie_remove_cheat(struct GameGenie* gg, gamegenie_id_t id)
{
    struct GameGenieEntry* prev = gg->entries;
    struct GameGenieEntry* entry = gg->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            prev->next = entry->next;
            free(entry);
            --gg->count;

            if (!gg->count)
            {
                gg->entries = NULL;
            }
            return;
        }
        else
        {
            prev = entry;
            entry = entry->next;
        }
    }
}

void gamegenie_reset(struct GameGenie* gg)
{
    struct GameGenieEntry* entry = gg->entries;

    while (entry != NULL)
    {
        struct GameGenieEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    memset(gg, 0, sizeof(struct GameGenie));
}

enum { SLOT_SIZE = 0x8000 };

// fair bit of code dupe below, will cleanup at some point.
bool gamegenie_apply_cheat(struct GameGenie* gg, uint8_t* rom, size_t len, gamegenie_id_t id)
{
    struct GameGenieEntry* entry = gg->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            // don't double set!
            if (entry->set)
            {
                return false;
            }

            for (size_t i = 0; i < len; i+= SLOT_SIZE)
            {
                uint8_t* rom_ptr = rom + i;

                switch (entry->type)
                {
                    case GameGenieType_1:
                        if (rom_ptr[entry->addr] == entry->compare)
                        {
                            entry->old_value = rom_ptr[entry->addr];
                            rom_ptr[entry->addr] = entry->new_value;
                            entry->set = true;
                        }
                        break;

                    case GameGenieType_2:
                        entry->old_value = rom_ptr[entry->addr];
                        rom_ptr[entry->addr] = entry->new_value;
                        entry->set = true;
                        break;
                }
            }

            return entry->set;
        }

        else
        {
            entry = entry->next;
        }
    }

    return false;
}

bool gamegenie_unapply_cheat(struct GameGenie* gg, uint8_t* rom, size_t len, gamegenie_id_t id)
{
    struct GameGenieEntry* entry = gg->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            // if its not set, we have no value to restore!
            if (entry->set == false)
            {
                return false;
            }

            for (size_t i = 0; i < len; i+= SLOT_SIZE)
            {
                uint8_t* rom_ptr = rom + i;

                rom_ptr[entry->addr] = entry->old_value;
                entry->set = false;
            }

            return !entry->set;
        }

        else
        {
            entry = entry->next;
        }
    }

    return false;
}
