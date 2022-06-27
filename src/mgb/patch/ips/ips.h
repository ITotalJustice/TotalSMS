#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


bool ips_verify_header(const uint8_t* patch, size_t patch_size);

/* dst_size is to be the same size as the src */
/* for this reason, it is allowed to alias as there is no point */
/* allocating a new buffer for a short patch */
bool ips_patch(
    uint8_t* dst, size_t dst_size,
    const uint8_t* src, size_t src_size,
    const uint8_t* patch, size_t patch_size
);

#ifdef __cplusplus
}
#endif
