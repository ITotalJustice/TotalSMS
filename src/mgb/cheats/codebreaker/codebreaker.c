#include "codebreaker.h"

#include <stdlib.h>
#include <string.h>


bool codebreaker_init(struct CodeBreaker* cb)
{
    if (!cb)
    {
        return false;
    }

    memset(cb, 0, sizeof(struct CodeBreaker));

    return true;
}

void codebreaker_exit(struct CodeBreaker* cb)
{
    codebreaker_reset(cb);
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

codebreaker_id_t codebreaker_add_cheat(struct CodeBreaker* cb, const char* s)
{
    const size_t len = strlen(s);

    // invalid length!
    if (len != 8)
    {
        return CODEBREAKER_INVALID_ID;
    }

    // 01,VV,AAAA
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
            return CODEBREAKER_INVALID_ID;
        }
    }

    const uint8_t   type    = hexv[0] << 4 | hexv[1];
    const uint8_t   value   = hexv[2] << 4 | hexv[3];
    const uint16_t  addr    = hexv[4] << 12 | hexv[5] << 8 | hexv[6] << 4 | hexv[7];

    if (type != 0x01)
    {
        return false;
    }

    codebreaker_id_t id = 0;

    for (size_t i = 0; i < sizeof(hexv); ++i)
    {
        id |= hexv[i] << (i * 4);
    } 

    struct CodeBreakerEntry entry = {0};
    entry.addr = addr;
    entry.value = value;
    entry.id = id;
    entry.enabled = true;

    struct CodeBreakerEntry* new_entry = (struct CodeBreakerEntry*)malloc(sizeof(struct CodeBreakerEntry));
    *new_entry = entry;

    if (cb->entries == NULL)
    {
        cb->entries = new_entry;
    }
    else
    {
        struct CodeBreakerEntry* _entry = cb->entries;

        while (_entry->next)
        {
            _entry = _entry->next;
        }

        _entry->next = new_entry;
    }

    cb->count++;

    return id;
}

void codebreaker_remove_cheat(struct CodeBreaker* cb, codebreaker_id_t id)
{
    struct CodeBreakerEntry* prev = cb->entries;
    struct CodeBreakerEntry* entry = cb->entries;

    while (entry != NULL)
    {
        if (entry->id == id)
        {
            prev->next = entry->next;
            free(entry);
            --cb->count;

            if (!cb->count)
            {
                cb->entries = NULL;
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

void codebreaker_reset(struct CodeBreaker* cb)
{
    struct CodeBreakerEntry* entry = cb->entries;

    while (entry != NULL)
    {
        struct CodeBreakerEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    memset(cb, 0, sizeof(struct CodeBreaker));
}

static bool toggle_cheat(struct CodeBreaker* cb, codebreaker_id_t id, bool enable)
{
    struct CodeBreakerEntry* entry = cb->entries;

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

bool codebreaker_enable_cheat(struct CodeBreaker* cb, codebreaker_id_t id)
{
    return toggle_cheat(cb, id, true);
}

bool codebreaker_disable_cheat(struct CodeBreaker* cb, codebreaker_id_t id)
{
    return toggle_cheat(cb, id, false);
}

void codebreaker_on_vblank(struct CodeBreaker* cb, void* user, cb_write_t write)
{
    struct CodeBreakerEntry* entry = cb->entries;

    while (entry != NULL)
    {
        if (entry->enabled)
        {
            write(user, entry->addr, entry->value);
        }

        entry = entry->next;
    }
}
