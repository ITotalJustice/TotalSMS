#include "cfile.h"
#include "../../util.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


enum LoadFileType
{
    LoadFileType_FILE,
    LoadFileType_FD,
};

struct Config
{
    const char* path;
    int fd;
    enum LoadFileType type;
};

#define PRIVATE_TO_CTX ctx_t* ctx = (ctx_t*)_private

#ifndef HAS_SDL2
    #define HAS_SDL2 0
#endif

// if we are using sdl, then might as well use sdlrwops
// the added benifit here rwops just work (tm) with android assets
#if HAS_SDL2
#include <SDL.h>

typedef SDL_RWops ctx_t;

static void iclose(void* _private) {
    PRIVATE_TO_CTX;
    SDL_RWclose(ctx);
}

static bool iread(void* _private, void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == SDL_RWread(ctx, data, 1, len);
}

static bool iwrite(void* _private, const void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == SDL_RWwrite(ctx, data, 1, len);
}

static bool iseek(void* _private, long offset, int whence) {
    PRIVATE_TO_CTX;
    return -1 != SDL_RWseek(ctx, offset, whence);
}

static size_t itell(void* _private) {
    PRIVATE_TO_CTX;

    const Sint64 result = SDL_RWtell(ctx);

    if (result < 0) {
        return 0;
    }

    return (size_t)result;
}

static size_t isize(void* _private) {
    PRIVATE_TO_CTX;

    const Sint64 file_size = SDL_RWsize(ctx);

    if (file_size < 0) {
        return 0;
    }

    return (size_t)file_size;
}

#else

#include <stdio.h>

typedef FILE ctx_t;

static void iclose(void* _private) {
    PRIVATE_TO_CTX;
    fclose(ctx);
}

static bool iread(void* _private, void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == fread(data, 1, len, ctx);
}

static bool iwrite(void* _private, const void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == fwrite(data, 1, len, ctx);
}

static bool iseek(void* _private, long offset, int whence) {
    PRIVATE_TO_CTX;
    return 0 == fseek(ctx, offset, whence);
}

static size_t itell(void* _private) {
    PRIVATE_TO_CTX;

    const long result = ftell(ctx);

    if (result < 0) {
        return 0;
    }

    return (size_t)result;
}

static size_t isize(void* _private) {
    PRIVATE_TO_CTX;

    const size_t old_offset = itell(ctx);
    iseek(ctx, 0, SEEK_END);
    const long file_size = itell(ctx);
    iseek(ctx, old_offset, SEEK_SET);

    if (file_size < 0) {
        return 0;
    }

    return (size_t)file_size;
}

#endif // HAS_SDL2

static ctx_t* internal_open_file(const char* file, const char* mode)
{
    #if HAS_SDL2
    return SDL_RWFromFile(file, mode);
    #else
    return fopen(file, mode);
    #endif
}

static ctx_t* internal_open_fd(int fd, const char* mode)
{
    FILE* f = fdopen(fd, mode);
    #if HAS_SDL2
    if (f)
    {
        return SDL_RWFromFP(f, true);
    }
    return NULL;
    #else
    return f;
    #endif
}

static IFile_t* internal_open(const struct Config* config, enum IFileMode mode, int flags)
{
    UNUSED(flags);

    IFile_t* ifile = NULL;
    ctx_t* ctx = NULL;

    ifile = (IFile_t*)malloc(sizeof(IFile_t));

    if (!ifile) {
        goto fail;
    }

    static const char* modes[] = {
        [IFileMode_READ] = "rb",
        [IFileMode_WRITE] = "wb",
    };

    switch (config->type)
    {
        case LoadFileType_FILE:
            ctx = internal_open_file(config->path, modes[mode]);
            break;

        case LoadFileType_FD:
            ctx = internal_open_fd(config->fd, modes[mode]);
            break;
    }

    if (!ctx) {
        goto fail;
    }

    const IFile_t _ifile = {
        ._private = ctx,
        .close  = iclose,
        .read   = iread,
        .write  = iwrite,
        .seek   = iseek,
        .tell   = itell,
        .size   = isize,
    };

    *ifile = _ifile;

    return ifile;

fail:
    if (ifile) {
        free(ifile);
    }

    return NULL;
}

IFile_t* icfile_open(const char* file, enum IFileMode mode, int flags)
{
    const struct Config config = {
        .path = file,
        .type = LoadFileType_FILE,
    };

    return internal_open(&config, mode, flags);
}

IFile_t* icfile_open_fd(int fd, bool own, enum IFileMode mode, int flags)
{
    if (!own) {
        fd = dup(fd);
    }

    const struct Config config = {
        .path = NULL,
        .fd = fd,
        .type = LoadFileType_FD,
    };

    return internal_open(&config, mode, flags);
}
