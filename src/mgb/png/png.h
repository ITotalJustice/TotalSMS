#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    PNG_HEADER_SIZE = 41,
    PNG_EOF_SIZE = 16,
};

bool png_is_valid(const void* data, size_t size);
size_t png_get_absolute_size(const void* data, size_t size);

uint8_t* png_uncompress(const void *img, size_t size, unsigned* w, unsigned* h, unsigned* len_out);
uint8_t* png_compress(const void *img, const unsigned w, const unsigned h, const int numchans, unsigned* len_out);

#ifdef __cplusplus
}
#endif
