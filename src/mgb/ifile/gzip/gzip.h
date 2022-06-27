#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../ifile.h"


IFile_t* igzip_open(const char* file, enum IFileMode mode, int flags);

#ifdef __cplusplus
}
#endif
