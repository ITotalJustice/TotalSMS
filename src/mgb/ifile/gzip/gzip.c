#include "gzip.h"
#include "../../util.h"
#include <stddef.h>
#include <stdlib.h>

#if 0
IFile_t* igzip_open(const char* file, enum IFileMode mode, int flags)
{
    return NULL;
}
#else
#include <zlib.h>


typedef struct {
    gzFile file;
} ctx_t;

#define PRIVATE_TO_CTX ctx_t* ctx = (ctx_t*)_private

static void iclose(void* _private) {
    PRIVATE_TO_CTX;
    gzclose(ctx->file);
}

static bool iread(void* _private, void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == (size_t)gzread(ctx->file, data, len);
}

static bool iwrite(void* _private, const void* data, size_t len) {
    PRIVATE_TO_CTX;
    return len == (size_t)gzwrite(ctx->file, data, len);
}

static bool iseek(void* _private, long offset, int whence) {
    PRIVATE_TO_CTX;
    return 1 != gzseek(ctx->file, offset, whence);
}

IFile_t* igzip_open(const char* path, enum IFileMode mode, int flags) {
    UNUSED(flags);

    IFile_t* ifile = NULL;
    ctx_t* ctx = NULL;
    gzFile file = NULL;

    ifile = malloc(sizeof(IFile_t));
    if (!ifile) {
        goto fail;
    }

    ctx = malloc(sizeof(ctx_t));
    if (!ctx) {
        goto fail;
    }

    const char* modes[] = {
        [IFileMode_READ] = "rb",
        [IFileMode_WRITE] = "wb",
    };

    file = gzopen(path, modes[mode]);
    if (!file) {
        goto fail;
    }

    const ctx_t _ctx = {
        .file = file,
    };

    *ctx = _ctx;

    const IFile_t _ifile = {
        ._private = ctx,
        .close  = iclose,
        .read   = iread,
        .write  = iwrite,
        .seek   = iseek,
        .tell   = NULL,
        .size   = NULL,
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
        gzclose(file);
        file = NULL;
    }

    return NULL;
}
#endif
