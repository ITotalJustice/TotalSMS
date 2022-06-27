// this uses stdlib for malloc and free!
// please call codebreaker_exit() before main exit!

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


typedef uint32_t codebreaker_id_t;
enum { CODEBREAKER_INVALID_ID = 0 };

struct CodeBreakerEntry
{
    struct CodeBreakerEntry* next;
    codebreaker_id_t id;

    uint16_t addr;
    uint8_t value;

    bool enabled;
};

struct CodeBreaker
{
    struct CodeBreakerEntry* entries;
    size_t count;
};

typedef void (*cb_write_t)(void* user, uint16_t addr, uint8_t value);


bool codebreaker_init(struct CodeBreaker* cb);
void codebreaker_exit(struct CodeBreaker* cb);

codebreaker_id_t codebreaker_add_cheat(struct CodeBreaker* cb, const char* s);
void codebreaker_remove_cheat(struct CodeBreaker* cb, codebreaker_id_t id);
void codebreaker_reset(struct CodeBreaker* cb);

bool codebreaker_enable_cheat(struct CodeBreaker* cb, codebreaker_id_t id);
bool codebreaker_disable_cheat(struct CodeBreaker* cb, codebreaker_id_t id);

// codebreaker cheats are applied at vblank
void codebreaker_on_vblank(struct CodeBreaker* cb, void* user, cb_write_t write);

#ifdef __cplusplus
}
#endif
