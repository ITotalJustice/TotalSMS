#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

size_t Zlib_size(size_t src_size);
size_t Zlib(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode);

size_t Zstd_size(size_t src_size);
size_t Zstd(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode);

size_t Lz4_size(size_t src_size);
size_t Lz4(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode);

#ifdef __cplusplus
}
#endif
