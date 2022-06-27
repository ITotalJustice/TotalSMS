#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../ifile.h"


IFile_t* icfile_open(const char* file, enum IFileMode mode, int flags);
IFile_t* icfile_open_fd(int fd, bool own, enum IFileMode mode, int flags);

#ifdef __cplusplus
}
#endif
