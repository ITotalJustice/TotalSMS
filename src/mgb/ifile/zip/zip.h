#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../ifile.h"


IFile_t* izip_open(const char* path, enum IFileMode mode, int flags);
IFile_t* izip_open_fd(int fd, bool own, enum IFileMode mode, int flags);
IFile_t* izip_open_mem(void* data, size_t size, enum IFileMode mode, int flags);
IFile_t* izip_open_mem_const(const void* data, size_t size, enum IFileMode mode, int flags);

#ifdef __cplusplus
}
#endif
