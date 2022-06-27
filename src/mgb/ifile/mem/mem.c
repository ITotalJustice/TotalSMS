#include "mem.h"
#include "../../util.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    void (*free)(void* data);
    union data
    {
        uint8_t* w;
        const uint8_t* r;
    } data;
    size_t offset;
    size_t size;
    size_t max_size;
    enum IFileMode mode;
} ctx_t;

#define PRIVATE_TO_CTX ctx_t* ctx = (ctx_t*)_private


static void internal_close(void* _private)
{
    PRIVATE_TO_CTX;

    if (ctx->data.w && ctx->free)
    {
        ctx->free(ctx->data.w);
        ctx->data.w = NULL;
        ctx->free = NULL;
    }

    free(ctx);
}

static bool internal_read(void* _private, void* data, size_t len)
{
    PRIVATE_TO_CTX;

    if (ctx->offset + len > ctx->size)
    {
        return false;
    }

    // avoid UB
    if (ctx->mode == IFileMode_READ)
    {
        memcpy(data, ctx->data.r + ctx->offset, len);
    }
    else
    {
        memcpy(data, ctx->data.w + ctx->offset, len);
    }

    ctx->offset += len;

    return true;
}

static bool internal_write(void* _private, const void* data, size_t len)
{
    PRIVATE_TO_CTX;

    // todo: handle memory growth if supported!
    if (ctx->offset + len > ctx->size)
    {
        return false;
    }

    // write func is disabled for const_open
    memcpy(ctx->data.w + ctx->offset, data, len);

    ctx->offset += len;

    return true;
}

static bool internal_seek(void* _private, long offset, int whence)
{
    PRIVATE_TO_CTX;

    switch (whence)
    {
        case 0:
            ctx->offset = offset;
            return true;

        case 1:
            ctx->offset += offset;
            return true;

        case 2:
            ctx->offset = ctx->size + offset;
            return true;
    }

    return false;
}

static size_t internal_tell(void* _private)
{
    PRIVATE_TO_CTX;

    return ctx->offset;
}

static size_t internal_size(void* _private)
{
    PRIVATE_TO_CTX;

    return ctx->size;
}

IFile_t* imem_open(void* data, size_t len, enum IFileMode mode, int flags)
{
    if (mode == IFileMode_READ)
    {
        return imem_open_const(data, len, mode, flags);
    }

    if (!data || !len)
    {
        return NULL;
    }

    IFile_t* ifile = (IFile_t*)malloc(sizeof(IFile_t));
    ctx_t* ctx = (ctx_t*)malloc(sizeof(ctx_t));

    const ctx_t _ctx =
    {
        .free = NULL,
        .data.w = (uint8_t*)data,
        .offset = 0,
        .size = len,
        .max_size = len,
        .mode = mode,
    };

    *ctx = _ctx;

    const IFile_t _ifile =
    {
        ._private = ctx,
        .close  = internal_close,
        .read   = internal_read,
        .write  = internal_write,
        .seek   = internal_seek,
        .tell   = internal_tell,
        .size   = internal_size,
    };

    *ifile = _ifile;

    return ifile;
}

IFile_t* imem_open_const(const void* data, size_t len, enum IFileMode mode, int flags)
{
    UNUSED(flags);

    if (!data || !len || mode == IFileMode_WRITE)
    {
        return NULL;
    }

    IFile_t* ifile = (IFile_t*)malloc(sizeof(IFile_t));
    ctx_t* ctx = (ctx_t*)malloc(sizeof(ctx_t));

    const ctx_t _ctx =
    {
        .free = NULL,
        .data.r = (const uint8_t*)data,
        .offset = 0,
        .size = len,
        .max_size = len,
        .mode = mode,
    };

    *ctx = _ctx;

    const IFile_t _ifile =
    {
        ._private = ctx,
        .close  = internal_close,
        .read   = internal_read,
        .write  = NULL,
        .seek   = internal_seek,
        .tell   = internal_tell,
        .size   = internal_size,
    };

    *ifile = _ifile;

    return ifile;
}
