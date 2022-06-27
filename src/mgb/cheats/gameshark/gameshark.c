#include "gameshark.h"

#include <stdlib.h>
#include <string.h>


bool gameshark_init(struct GameShark* gs)
{
    if (!gs)
    {
        return false;
    }

    memset(gs, 0, sizeof(struct GameShark));

    return true;
}

void gameshark_exit(struct GameShark* gs)
{
    gameshark_reset(gs);
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

gameshark_id_t gameshark_add_cheat(struct GameShark* gs, const char* s)
{
    const size_t len = strlen(s);

    // invalid length!
    if (len != 8)
    {
        return GAMESHARK_INVALID_ID;
    }

    // 0B,VV,AAAA
    // B = 4-bit bank
    // V = 8-bit value
    // A = 16-bit addr

    uint8_t hexv[8];

    for (size_t i = 0; i < sizeof(hexv); ++i)
    {
        hexv[i] = hex(s[i]);

        // check that all the hex values were valid.
        // the hex func returns a nibble (4-bits) at a time
        // so any number greater than 0xF is invalid.
        if (hexv[i] > 0xF)
        {
            return GAMESHARK_INVALID_ID;
        }
    }

    const uint8_t   type    = hexv[0] << 4 | hexv[1];
    const uint8_t   bank    = hexv[1];
    const uint8_t   value   = hexv[2] << 4 | hexv[3];
    const uint16_t  addr    = hexv[6] << 12 | hexv[7] << 8 | hexv[4] << 4 | hexv[5];

    gameshark_id_t id = 0;

    for (size_t i = 0; i < sizeof(hexv); ++i)
    {
        id |= hexv[i] << (i * 4);
    } 

    struct GameSharkEntry entry = {0};
    entry.addr = addr;
    entry.bank = bank;
    entry.value = value;
    entry.id = id;
    entry.enabled = true;

    // there are 3 types or cheats!
    if (type == 0x01)
    {
        entry.type = GamesharkType_WRITE;
    }
    else if ((type & 0x80) == 0x80)
    {
        entry.type = GamesharkType_WRITE_WITH_RAM_BANK_CHANGE;
    }
    else if ((type & 0x90) == 0x90)
    {
        entry.type = GamesharkType_WRITE_WITH_WRAM_BANK_CHANGE;
    }
    else
    {
        return GAMESHARK_INVALID_ID;
    }

    struct GameSharkEntry* new_entry = (struct GameSharkEntry*)malloc(sizeof(struct GameSharkEntry));
    *new_entry = entry;

    if (gs->entries == NULL)
    {
        gs->entries = new_entry;
    }
    else
    {
        struct GameSharkEntry* _entry = gs->entries;

        while (_entry->next)
        {
            _entry = _entry->next;
        }

        _entry->next = new_entry;
    }

    gs->count++;

    return id;
}

void gameshark_remove_cheat(struct GameShark* gs, gameshark_id_t id)
{
    struct GameSharkEntry* prev = gs->entries;
    struct GameSharkEntry* entry = gs->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            prev->next = entry->next;
            free(entry);
            --gs->count;

            if (!gs->count)
            {
                gs->entries = NULL;
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

void gameshark_reset(struct GameShark* gs)
{
    struct GameSharkEntry* entry = gs->entries;

    while (entry != NULL)
    {
        struct GameSharkEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    memset(gs, 0, sizeof(struct GameShark));
}

static bool toggle_cheat(struct GameShark* gs, gameshark_id_t id, bool enable)
{
    struct GameSharkEntry* entry = gs->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            entry->enabled = enable;
            return true;
        }
        else
        {
            entry = entry->next;
        }
    }

    return false;
}

bool gameshark_enable_cheat(struct GameShark* gs, gameshark_id_t id)
{
    return toggle_cheat(gs, id, true);
}

bool gameshark_disable_cheat(struct GameShark* gs, gameshark_id_t id)
{
    return toggle_cheat(gs, id, false);
}

void gameshark_on_vblank(struct GameShark* gs, void* user,
    gs_write_t write,
    gs_set_ram_bank_t set_ram_bank,
    gs_set_wram_bank_t set_wram_bank
) {
    struct GameSharkEntry* entry = gs->entries;

    while (entry != NULL)
    {
        if (entry->enabled)
        {
            switch (entry->type)
            {
                case GamesharkType_WRITE:
                    write(user, entry->addr, entry->value);
                    break;

                case GamesharkType_WRITE_WITH_RAM_BANK_CHANGE:
                    set_ram_bank(user, entry->bank);
                    write(user, entry->addr, entry->value);
                    break;

                case GamesharkType_WRITE_WITH_WRAM_BANK_CHANGE:
                    set_wram_bank(user, entry->bank);
                    write(user, entry->addr, entry->value);
                    break;       
            }
        }

        entry = entry->next;
    }
}
