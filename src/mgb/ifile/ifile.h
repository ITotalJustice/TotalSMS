// todo: the open func should take an enum
// of ifile_type which will call that type's function
// that way, only 1 header needs to be included, and
// only 1 function call is used across the codebase!

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


enum IFileMode {
    IFileMode_READ,
    IFileMode_WRITE,
};

enum IFileWhence {
    IFileWhence_SET,
    IFileWhence_CUR,
    IFileWhence_END,
};

typedef struct IFile {
    void* _private;

    void (*close)(void* _private);
    bool (*read)(void* _private, void* data, size_t len);
    bool (*write)(void* _private, const void* data, size_t len);
    bool (*seek)(void* _private, long offset, int whence);
    size_t (*tell)(void* _private);
    size_t (*size)(void* _private);
} IFile_t;

void ifile_close(IFile_t* ifile);
bool ifile_read(IFile_t* ifile, void* data, size_t len);
bool ifile_write(IFile_t* ifile, const void* data, size_t len);
bool ifile_seek(IFile_t* ifile, long offset, int whence);
size_t ifile_tell(IFile_t* ifile);
size_t ifile_size(IFile_t* ifile);

#ifdef __cplusplus
}
#endif
