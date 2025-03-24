#include "png.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static void crc32_to_buf(uint8_t* buf, const uint8_t* in, size_t len) {
    const uint32_t c = crc32(0, in, len);
    buf[0] = c >> 24;
    buf[1] = c >> 16;
    buf[2] = c >> 8;
    buf[3] = c >> 0;
}

bool png_is_valid(const void* data, size_t size) {
    if (!data) {
        return false;
    }

    if (size < PNG_HEADER_SIZE) {
        return false;
    }

    // check if we have the header signature
    const uint8_t sig[0x8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    return memcmp(data, sig, sizeof(sig)) == 0;
}

size_t png_get_absolute_size(const void* data, size_t size) {
    if (!png_is_valid(data, size)) {
        return 0;
    }

    size_t png_data_size = 0;
    png_data_size |= ((const uint8_t*)data)[33] << 24;
    png_data_size |= ((const uint8_t*)data)[34] << 16;
    png_data_size |= ((const uint8_t*)data)[35] << 8;
    png_data_size |= ((const uint8_t*)data)[36];

    return png_data_size + PNG_HEADER_SIZE + PNG_EOF_SIZE;
}

// todo: actually check png size, currently assuming rgb24
uint8_t* png_uncompress(const void *img, size_t size, unsigned* w_out, unsigned* h_out, unsigned* len_out) {
    if (!png_is_valid(img, size)) {
        return NULL;
    }

    const uint8_t* pnghdr = (const uint8_t*)img;
    const unsigned w = (pnghdr[18] << 8) | pnghdr[19];
    const unsigned h = (pnghdr[22] << 8) | pnghdr[23];
    *len_out = (pnghdr[33] << 24) | (pnghdr[34] << 16) | (pnghdr[35] << 8) | pnghdr[36];
    if (!*len_out || !w || !h) {
        return NULL;
    }

    unsigned numchans = 0;
    switch (pnghdr[25]) {
        case 0:
            return NULL; // unsupported
        case 2:
            numchans = 3;
            break;
        case 3:
            return NULL; // unsupported
        case 4:
            return NULL; // unsupported
        case 6:
            numchans = 4;
            break;
    }

    const unsigned p = w * numchans;

    z_stream z = {0};
    z.avail_in = p * h;
    z.next_in = (z_const Bytef*)img + PNG_HEADER_SIZE; // skip header
    if (Z_OK != inflateInit(&z)) {
        return NULL;
    }

    *len_out = p * h;
    uint8_t* zbuf = malloc(*len_out);
    if (!zbuf) {
        inflateEnd(&z);
        return NULL;
    }

    for (unsigned y = 0; y < h; y++) {
        Bytef filter = 0;
        z.avail_out = 1;
        z.next_out = &filter;
        inflate(&z, Z_NO_FLUSH);

        z.next_out = zbuf + y * p;
        z.avail_out = p;
        const int ret = inflate(&z, Z_NO_FLUSH);
        if (Z_OK != ret) {
            if (ret == Z_STREAM_END) {
                break;
            } else {
                inflateEnd(&z);
                free(zbuf);
                return NULL;
            }
        }
    }

    if (Z_OK != inflateEnd(&z)) {
        free(zbuf);
        return NULL;
    }

    *w_out = w;
    *h_out = h;
    return zbuf;
}

// https://gist.github.com/mmalex/908299
uint8_t* png_compress(const void *img, const unsigned w, const unsigned h, const int numchans, unsigned* len_out) {
    const unsigned p = w * numchans;
    z_stream z = {0};
    if (Z_OK != deflateInit(&z, Z_DEFAULT_COMPRESSION)) {
        return NULL;
    }

    uint8_t* zbuf = malloc(PNG_HEADER_SIZE + PNG_EOF_SIZE + (z.avail_out = deflateBound(&z, (1+p)*h)) + 1);
    if (!zbuf) {
        deflateEnd(&z);
        return NULL;
    }

    z.next_out = zbuf + PNG_HEADER_SIZE;

    for (unsigned y = 0; y < h; y++) {
        Bytef filter = 0;
        z.avail_in = 1;
        z.next_in = &filter;
        deflate(&z, Z_NO_FLUSH);

        z.avail_in = p;
        z.next_in = (z_const Bytef*)img + y * p;
        const int ret = deflate(&z,(y == h - 1) ? Z_FINISH : Z_NO_FLUSH);
        if (Z_OK != ret) {
            if (ret == Z_STREAM_END) {
                break;
            } else {
                deflateEnd(&z);
                free(zbuf);
                return NULL;
            }
        }
    }

    if (Z_OK != deflateEnd(&z)) {
        free(zbuf);
        return NULL;
    }

    *len_out = z.next_out - zbuf - PNG_HEADER_SIZE;

    const uint8_t chans[] = { 0x00, 0x00, 0x04, 0x02, 0x06 };
    uint8_t pnghdr[PNG_HEADER_SIZE] = { 0x89, 0x50, 0x4e, 0x47, 0x0d,
        0x0a, 0x1a, 0x0a, 0x00, 0x00,
        0x00, 0x0d, 0x49, 0x48, 0x44,
        0x52, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x08,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x49, 0x44, 0x41,
        0x54
    };

    pnghdr[16] = w >> 24;
    pnghdr[17] = w >> 16;
    pnghdr[18] = w >> 8;
    pnghdr[19] = w;
    pnghdr[20] = h >> 24;
    pnghdr[21] = h >> 16;
    pnghdr[22] = h >> 8;
    pnghdr[23] = h;
    pnghdr[25] = chans[numchans]; // colour type
    pnghdr[33] = *len_out >> 24;
    pnghdr[34] = *len_out >> 16;
    pnghdr[35] = *len_out >> 8;
    pnghdr[36] = *len_out;

    crc32_to_buf(pnghdr + 29, pnghdr + 12, 17);
    memcpy(zbuf, pnghdr, PNG_HEADER_SIZE);
    memcpy(z.next_out + 4, "\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 12); /* footer */
    crc32_to_buf(z.next_out, zbuf+PNG_HEADER_SIZE-4, *len_out+4);
    *len_out += PNG_HEADER_SIZE + PNG_EOF_SIZE;

    return zbuf;
}
