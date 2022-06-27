#pragma once

#include "util.h"
#include <stddef.h>

enum DirectoryFlags
{
    DIRECTORY_READ = 1 << 0,
    DIRECTORY_WRITE = 1 << 1,
    DIRECTORY_EXECUTE = 1 << 2,

    DIRECTORY_RW = DIRECTORY_READ | DIRECTORY_WRITE,
    DIRECTORY_RWX = DIRECTORY_READ | DIRECTORY_WRITE | DIRECTORY_EXECUTE,
};

enum DirEntryType
{
    DirEntryType_FILE,
    DirEntryType_FOLDER,
    DirEntryType_UNK = 0xFF,
};

struct DirEntry
{
    char name[256];
    enum DirEntryType type;
};

// returns the number of entries found.
// this function can be slow, so it's best to call it from another thread and then wait
// for the result.
size_t directory_scan(const char* path, struct DirEntry* entries, size_t count);

// written in the style of sdl2 source as i hope to
// have this function merged into sdl2 :)
int directory_create(const char* path, uint32_t flags);
