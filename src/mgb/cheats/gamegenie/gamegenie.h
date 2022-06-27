// this uses stdlib for malloc and free!
// please call gamegenie_exit() before main exit!

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


typedef uint32_t gamegenie_id_t;
enum { GAMEGENIE_INVALID_ID = 0 };

enum GameGenieType
{
    GameGenieType_1 = 11,   // "000-000-000"
    GameGenieType_2 = 7,    // "000-000"
};

struct GameGenieEntry
{
    struct GameGenieEntry* next;
    gamegenie_id_t id;
    enum GameGenieType type;

    uint16_t addr;
    uint8_t new_value;
    uint8_t old_value;
    uint8_t compare;

    bool set; // is the cheat set
};

struct GameGenie
{
    struct GameGenieEntry* entries;
    size_t count;
};


bool gamegenie_init(struct GameGenie* gg);
void gamegenie_exit(struct GameGenie* gg);

gamegenie_id_t gamegenie_add_cheat(struct GameGenie* gg, const char* s);
void gamegenie_remove_cheat(struct GameGenie* gg, gamegenie_id_t id);
void gamegenie_reset(struct GameGenie* gg);

bool gamegenie_apply_cheat(struct GameGenie* gg, uint8_t* rom, size_t len, gamegenie_id_t id);
bool gamegenie_unapply_cheat(struct GameGenie* gg, uint8_t* rom, size_t len, gamegenie_id_t id);

#ifdef __cplusplus
}
#endif
