#if 0
#include <zstd.h>
#include <lz4.h>
#endif
#include <zlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


enum CompressMode
{
    CompressMode_DEFLATE,
    CompressMode_INFLATE,
};

size_t Zlib_size(size_t src_size)
{
    return compressBound(src_size);
}

size_t Zlib(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode)
{
    int result = 0;
    uLongf dst_size_ = 0; // silence warn for ulong being different size to size_t

    if (mode == CompressMode_DEFLATE)
    {
        result = compress(dst_data, &dst_size_, src_data, src_size);
    }
    else
    {
        result = uncompress(dst_data, &dst_size_, src_data, src_size);
    }
    dst_size = dst_size_;

    return result == Z_OK ? dst_size : (size_t) - 1;
}

#if 0
size_t Zstd_size(size_t src_size)
{
    return ZSTD_compressBound(src_size);
}

size_t Zstd(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode)
{
    if (mode == CompressMode_DEFLATE)
    {
        return ZSTD_compress(dst_data, dst_size, src_data, src_size, ZSTD_CLEVEL_DEFAULT);
    }
    else
    {
        return ZSTD_decompress(dst_data, dst_size, src_data, src_size);
    }
}

size_t Lz4_size(size_t src_size)
{
    return LZ4_compressBound(src_size);
}

size_t Lz4(void* dst_data, size_t dst_size, const void* src_data, size_t src_size, int mode)
{
    if (mode == CompressMode_DEFLATE)
    {
        return LZ4_compress_default(src_data, dst_data, src_size, dst_size);
    }
    else
    {
        return LZ4_decompress_safe(src_data, dst_data, src_size, dst_size);
    }
}
#endif
