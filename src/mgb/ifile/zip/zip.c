#include "zip.h"
#include "../../mgb.h"
#include "../../util.h"
#include <stdio.h>

enum LoadFileType
{
    LoadFileType_FILE,
    LoadFileType_FD,
};

typedef struct
{
    const char* path;
    int fd;
    enum LoadFileType type;
} config_t;

#if 0
IFile_t* izip_open(const char* path, enum IFileMode mode, int flags)
{
    return NULL;
}
IFile_t* izip_open_mem(void* data, size_t size, enum IFileMode mode, int flags)
{
    return NULL;
}
#else
#include <zip.h>
#include <unzip.h>
#include <ioapi_mem.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef HAS_SDL2
#include <SDL.h>

static const char* sdl_get_mode_from_zlib(int mode) {
    const char* mode_str = NULL;

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
    {
        mode_str = "rb";
    }
    else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        mode_str = "r+b";
    }
    else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        mode_str = "wb";
    }

    return mode_str;
}

static voidpf sdl_open_file(voidpf opaque, const char *filename, int mode)
{
    UNUSED(opaque);
    const char* mode_str = sdl_get_mode_from_zlib(mode);
    return SDL_RWFromFile(filename, mode_str);
}

// todo: support write mode!
static voidpf sdl_open_fd(voidpf opaque, const char *filename, int mode)
{
    UNUSED(opaque); UNUSED(filename); UNUSED(mode);
    return SDL_RWFromFP((FILE*)opaque, true);
}

static voidpf sdl_open_disk(voidpf opaque, voidpf stream, uint32_t number_disk, int mode)
{
    UNUSED(opaque); UNUSED(stream); UNUSED(number_disk); UNUSED(mode);
    mgb_log_err("trying to open disk - disk_number: %u mode: %d\n", number_disk, mode);
    return NULL;
}

static uint32_t sdl_read_file(voidpf opaque, voidpf stream, void* buf, uint32_t size)
{
    UNUSED(opaque);
    return SDL_RWread((SDL_RWops*)stream, buf, 1, size);
}

static uint32_t sdl_write_file(voidpf opaque, voidpf stream, const void *buf, uint32_t size)
{
    UNUSED(opaque);
    return SDL_RWwrite((SDL_RWops*)stream, buf, 1, size);
}

static int sdl_close_file(voidpf opaque, voidpf stream)
{
    UNUSED(opaque);
    return SDL_RWclose((SDL_RWops*)stream);
}

static int sdl_error_file(voidpf opaque, voidpf stream)
{
    UNUSED(opaque); UNUSED(stream);
    mgb_log_err("we got an sdl file error\n");
    return 0;
}

static long sdl_tell_file(voidpf opaque, voidpf stream)
{
    UNUSED(opaque);
    return SDL_RWtell((SDL_RWops*)stream);
}

static long sdl_seek_file(voidpf opaque, voidpf stream, uint32_t offset, int origin)
{
    UNUSED(opaque);
    const int64_t r = SDL_RWseek((SDL_RWops*)stream, offset, origin);
    return r == -1 ? -1 : 0;
}

static zlib_filefunc_def SDL_FILEFUNC =
{
    .zopen_file = sdl_open_file,
    .zopendisk_file = sdl_open_disk,
    .zread_file = sdl_read_file,
    .zwrite_file = sdl_write_file,
    .ztell_file = sdl_tell_file,
    .zseek_file = sdl_seek_file,
    .zclose_file = sdl_close_file,
    .zerror_file = sdl_error_file,
    .opaque = NULL,
};
#endif

typedef struct {
    union {
        unzFile u;
        zipFile z;
    } file;
    ourmemory_t* ourmem;
    enum IFileMode mode;
} ctx_t;

#define PRIVATE_TO_CTX ctx_t* ctx = (ctx_t*)_private
#define IFILE_TO_CTX ctx_t* ctx = (ctx_t*)ifile->_private

static bool find_and_open_file_type(IFile_t* ifile, int flags) {
    IFILE_TO_CTX;

    if (ctx->mode != IFileMode_READ) {
        return false;
    }

    unz_file_info info = {0};
    char name[256] = {0};

    do {
        if (UNZ_OK == unzGetCurrentFileInfo(ctx->file.u, &info, name, sizeof(name), NULL, 0, NULL, 0)) {
            if (util_is_extension(name, ExtensionOffsetType_LAST, flags)) {
                if (UNZ_OK == unzOpenCurrentFile(ctx->file.u)) {
                    return true;
                } else {
                    mgb_log_err("found wanted file %s in zip, but failed to open it!\n", name);
                }
            }
        }
    } while (UNZ_OK == unzGoToNextFile(ctx->file.u));

    return false;
}

static void iclose(void* _private) {
    PRIVATE_TO_CTX;

    if (ctx) {
        switch (ctx->mode) {
            case IFileMode_READ:
                if (ctx->file.u) {
                    unzCloseCurrentFile(ctx->file.u);
                    unzClose(ctx->file.u);
                    ctx->file.u = NULL;
                }
                break;

            case IFileMode_WRITE:
                if (ctx->file.z) {
                    zipCloseFileInZip(ctx->file.z);
                    zipClose(ctx->file.z, NULL);
                    ctx->file.z = NULL;
                }
                break;
        }

        if (ctx->ourmem) {
            // do NOT free this, unless we own the data
            // that we passed.
            if (ctx->ourmem->grow) {
                free(ctx->ourmem->data.w);
                ctx->ourmem->data.w = NULL;
            }
            free(ctx->ourmem);
            ctx->ourmem = NULL;
        }

        free(ctx);
        ctx = NULL;
    }
}

static bool iread(void* _private, void* data, size_t len) {
    PRIVATE_TO_CTX;

    if (ctx->mode != IFileMode_READ) {
        return false;
    }

    return 0 < unzReadCurrentFile(ctx->file.u, data, len);
}

static bool iwrite(void* _private, const void* data, size_t len) {
    PRIVATE_TO_CTX;

    if (ctx->mode != IFileMode_WRITE) {
        return false;
    }

    return 0 < zipWriteInFileInZip(ctx->file.z, data, len);
}

static bool iseek(void* _private, long offset, int whence) {
    PRIVATE_TO_CTX;

    if (ctx->mode != IFileMode_READ) {
        return false;
    }

    // cannot seek backwards!
    if (offset < 0) {
        return false;
    }

    return UNZ_OK == unzSeek(ctx->file.u, (uint32_t)offset, whence);
}

static size_t itell(void* _private) {
    PRIVATE_TO_CTX;

    if (ctx->mode != IFileMode_READ) {
        return 0;
    }

    // returns uncompressed offset
    const int32_t r = unzTell(ctx->file.u);
    return r < 0 ? 0 : r;
}

static size_t isize(void* _private) {
    PRIVATE_TO_CTX;

    if (ctx->mode != IFileMode_READ) {
        return false;
    }

    unz_file_info info = {0};

    const int result = unzGetCurrentFileInfo(
        ctx->file.u, &info,
        NULL, 0, NULL, 0, NULL, 0
    );

    if (result != UNZ_OK) {
        return 0;
    }

    return info.uncompressed_size;
}

static IFile_t* internal_open_read(const config_t* config, int flags) {
    IFile_t* ifile = NULL;
    ctx_t* ctx = NULL;
    unzFile file = NULL;

    ifile = (IFile_t*)malloc(sizeof(IFile_t));
    if (!ifile) {
        goto fail;
    }

    ctx = (ctx_t*)malloc(sizeof(ctx_t));
    if (!ctx) {
        goto fail;
    }

    zlib_filefunc_def filefunc_def = {0};
#ifdef HAS_SDL2
    filefunc_def = SDL_FILEFUNC;
#else
    fill_fopen_filefunc(&filefunc_def);
#endif

    switch (config->type) {
        case LoadFileType_FILE:
            file = unzOpen2(config->path, &filefunc_def);
            break;

        case LoadFileType_FD:
            #ifdef HAS_SDL2
                filefunc_def.zopen_file = sdl_open_fd;
            #else
            #endif
            filefunc_def.opaque = fdopen(config->fd, "rb");
            file = unzOpen2("__notused__", &filefunc_def);
            break;
    }

    if (!file) {
        goto fail;
    }

    const ctx_t _ctx = {
        .file.u = file,
        .ourmem = NULL, // unused!
        .mode = IFileMode_READ
    };

    *ctx = _ctx;

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

    if (!find_and_open_file_type(ifile, flags)) {
        goto fail;
    }

    return ifile;

fail:
    if (ifile) {
        free(ifile);
        ifile = NULL;
    }

    if (ctx) {
        free(ctx);
        ctx = NULL;
    }

    if (file) {
        unzClose(file);
        file = NULL;
    }

    return NULL;
}

static IFile_t* internal_open_read_file(const char* path, int flags) {
    const config_t config = {
        .path = path,
        .type = LoadFileType_FILE,
    };
    return internal_open_read(&config, flags);
}

static IFile_t* internal_open_read_fd(int fd, int flags) {
    const config_t config = {
        .fd = fd,
        .type = LoadFileType_FD,
    };
    return internal_open_read(&config, flags);
}

static IFile_t* open_write(const char* path, int flags) {
    UNUSED(flags);

    IFile_t* ifile = NULL;
    ctx_t* ctx = NULL;
    unzFile file = NULL;

    ifile = (IFile_t*)malloc(sizeof(IFile_t));
    if (!ifile) {
        goto fail;
    }

    ctx = (ctx_t*)malloc(sizeof(ctx_t));
    if (!ctx) {
        goto fail;
    }

    file = zipOpen(path, APPEND_STATUS_CREATE);
    if (!file) {
        goto fail;
    }

    const ctx_t _ctx = {
        .file.z = file,
        .ourmem = NULL, // unused!
        .mode = IFileMode_WRITE
    };

    *ctx = _ctx;

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
        ifile = NULL;
    }

    if (ctx) {
        free(ctx);
        ctx = NULL;
    }

    if (file) {
        unzClose(file);
        file = NULL;
    }

    return NULL;
}

IFile_t* izip_open(const char* path, enum IFileMode mode, int flags) {
    switch (mode) {
        case IFileMode_READ:
            return internal_open_read_file(path, flags);

        case IFileMode_WRITE:
            return open_write(path, flags);
    }

    return NULL;
}

#if defined(VITA) || defined(PSP) || defined(PS2) || defined(DREAMCAST)
int dup(int fd)
{
    return -1;
}
#endif

IFile_t* izip_open_fd(int fd, bool own, enum IFileMode mode, int flags) {
    if (!own) {
        fd = dup(fd);
    }

    switch (mode) {
        case IFileMode_READ:
            return internal_open_read_fd(fd, flags);

        case IFileMode_WRITE:
            return NULL;
    }

    return NULL;
}

union Data
{
    void* w;
    const void* r;
};

static IFile_t* open_read_mem(union Data data, size_t size, enum IFileMode mode, int flags) {
    IFile_t* ifile = NULL;
    ctx_t* ctx = NULL;
    unzFile file = NULL;
    ourmemory_t* ourmemory = NULL;
    zlib_filefunc_def filefunc32 = {0};

    ifile = (IFile_t*)malloc(sizeof(IFile_t));
    if (!ifile) {
        goto fail;
    }

    ctx = (ctx_t*)malloc(sizeof(ctx_t));
    if (!ctx) {
        goto fail;
    }

    ourmemory = (ourmemory_t*)malloc(sizeof(ourmemory_t));
    if (!ourmemory) {
        goto fail;
    }

    switch (mode) {
        case IFileMode_READ:
            ourmemory->size = size;
            ourmemory->data.r = (const char*)data.r;
            fill_memory_filefunc_const(&filefunc32, ourmemory);
            break;

        case IFileMode_WRITE:
            ourmemory->size = size;
            ourmemory->data.w = (char*)data.w;
            fill_memory_filefunc(&filefunc32, ourmemory);
            break;
    }

    file = unzOpen2("__notused__", &filefunc32);
    if (!file) {
        goto fail;
    }

    const ctx_t _ctx = {
        .file.u = file,
        .ourmem = ourmemory,
        .mode = IFileMode_READ
    };

    *ctx = _ctx;

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

    if (!find_and_open_file_type(ifile, flags)) {
        mgb_log_err("failed to find rom!\n");
        goto fail;
    }

    return ifile;

fail:
    if (ifile) {
        free(ifile);
        ifile = NULL;
    }

    if (ctx) {
        free(ctx);
        ctx = NULL;
    }

    if (ourmemory) {
        free(ourmemory);
        ourmemory = NULL;
    }

    if (file) {
        unzClose(file);
        file = NULL;
    }

    return NULL;
}

IFile_t* izip_open_mem(void* data, size_t size, enum IFileMode mode, int flags) {
    union Data d = {0};

    switch (mode) {
        case IFileMode_READ:
            return NULL;

        case IFileMode_WRITE:
            d.w = data;
            return open_read_mem(d, size, mode, flags);
    }

    return NULL;
}

IFile_t* izip_open_mem_const(const void* data, size_t size, enum IFileMode mode, int flags) {
    union Data d = {0};

    switch (mode) {
        case IFileMode_READ:
            d.r = data;
            return open_read_mem(d, size, mode, flags);

        case IFileMode_WRITE:
            return NULL;
    }

    return NULL;
}
#endif
