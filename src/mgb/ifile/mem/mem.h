#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../ifile.h"


// this acts as a view of the data, it does NOT free the memory after!
IFile_t* imem_open(void* data, size_t len, enum IFileMode mode, int flags);
IFile_t* imem_open_const(const void* data, size_t len, enum IFileMode mode, int flags);

#ifdef __cplusplus
}
#endif
