#include "ups.h"


/* header is 4 bytes plus at least 2 bytes for input and output size */
#define PATCH_HEADER_SIZE 0x4
#define PATCH_MIN_SIZE 0x6


static const uint32_t CRC32_TABLE[0x100] =
{
    0xD202EF8D, 0xA505DF1B, 0x3C0C8EA1, 0x4B0BBE37, 0xD56F2B94, 0xA2681B02, 0x3B614AB8, 0x4C667A2E,
    0xDCD967BF, 0xABDE5729, 0x32D70693, 0x45D03605, 0xDBB4A3A6, 0xACB39330, 0x35BAC28A, 0x42BDF21C,
    0xCFB5FFE9, 0xB8B2CF7F, 0x21BB9EC5, 0x56BCAE53, 0xC8D83BF0, 0xBFDF0B66, 0x26D65ADC, 0x51D16A4A,
    0xC16E77DB, 0xB669474D, 0x2F6016F7, 0x58672661, 0xC603B3C2, 0xB1048354, 0x280DD2EE, 0x5F0AE278,
    0xE96CCF45, 0x9E6BFFD3, 0x0762AE69, 0x70659EFF, 0xEE010B5C, 0x99063BCA, 0x000F6A70, 0x77085AE6,
    0xE7B74777, 0x90B077E1, 0x09B9265B, 0x7EBE16CD, 0xE0DA836E, 0x97DDB3F8, 0x0ED4E242, 0x79D3D2D4,
    0xF4DBDF21, 0x83DCEFB7, 0x1AD5BE0D, 0x6DD28E9B, 0xF3B61B38, 0x84B12BAE, 0x1DB87A14, 0x6ABF4A82,
    0xFA005713, 0x8D076785, 0x140E363F, 0x630906A9, 0xFD6D930A, 0x8A6AA39C, 0x1363F226, 0x6464C2B0,
    0xA4DEAE1D, 0xD3D99E8B, 0x4AD0CF31, 0x3DD7FFA7, 0xA3B36A04, 0xD4B45A92, 0x4DBD0B28, 0x3ABA3BBE,
    0xAA05262F, 0xDD0216B9, 0x440B4703, 0x330C7795, 0xAD68E236, 0xDA6FD2A0, 0x4366831A, 0x3461B38C,
    0xB969BE79, 0xCE6E8EEF, 0x5767DF55, 0x2060EFC3, 0xBE047A60, 0xC9034AF6, 0x500A1B4C, 0x270D2BDA,
    0xB7B2364B, 0xC0B506DD, 0x59BC5767, 0x2EBB67F1, 0xB0DFF252, 0xC7D8C2C4, 0x5ED1937E, 0x29D6A3E8,
    0x9FB08ED5, 0xE8B7BE43, 0x71BEEFF9, 0x06B9DF6F, 0x98DD4ACC, 0xEFDA7A5A, 0x76D32BE0, 0x01D41B76,
    0x916B06E7, 0xE66C3671, 0x7F6567CB, 0x0862575D, 0x9606C2FE, 0xE101F268, 0x7808A3D2, 0x0F0F9344,
    0x82079EB1, 0xF500AE27, 0x6C09FF9D, 0x1B0ECF0B, 0x856A5AA8, 0xF26D6A3E, 0x6B643B84, 0x1C630B12,
    0x8CDC1683, 0xFBDB2615, 0x62D277AF, 0x15D54739, 0x8BB1D29A, 0xFCB6E20C, 0x65BFB3B6, 0x12B88320,
    0x3FBA6CAD, 0x48BD5C3B, 0xD1B40D81, 0xA6B33D17, 0x38D7A8B4, 0x4FD09822, 0xD6D9C998, 0xA1DEF90E,
    0x3161E49F, 0x4666D409, 0xDF6F85B3, 0xA868B525, 0x360C2086, 0x410B1010, 0xD80241AA, 0xAF05713C,
    0x220D7CC9, 0x550A4C5F, 0xCC031DE5, 0xBB042D73, 0x2560B8D0, 0x52678846, 0xCB6ED9FC, 0xBC69E96A,
    0x2CD6F4FB, 0x5BD1C46D, 0xC2D895D7, 0xB5DFA541, 0x2BBB30E2, 0x5CBC0074, 0xC5B551CE, 0xB2B26158,
    0x04D44C65, 0x73D37CF3, 0xEADA2D49, 0x9DDD1DDF, 0x03B9887C, 0x74BEB8EA, 0xEDB7E950, 0x9AB0D9C6,
    0x0A0FC457, 0x7D08F4C1, 0xE401A57B, 0x930695ED, 0x0D62004E, 0x7A6530D8, 0xE36C6162, 0x946B51F4,
    0x19635C01, 0x6E646C97, 0xF76D3D2D, 0x806A0DBB, 0x1E0E9818, 0x6909A88E, 0xF000F934, 0x8707C9A2,
    0x17B8D433, 0x60BFE4A5, 0xF9B6B51F, 0x8EB18589, 0x10D5102A, 0x67D220BC, 0xFEDB7106, 0x89DC4190,
    0x49662D3D, 0x3E611DAB, 0xA7684C11, 0xD06F7C87, 0x4E0BE924, 0x390CD9B2, 0xA0058808, 0xD702B89E,
    0x47BDA50F, 0x30BA9599, 0xA9B3C423, 0xDEB4F4B5, 0x40D06116, 0x37D75180, 0xAEDE003A, 0xD9D930AC,
    0x54D13D59, 0x23D60DCF, 0xBADF5C75, 0xCDD86CE3, 0x53BCF940, 0x24BBC9D6, 0xBDB2986C, 0xCAB5A8FA,
    0x5A0AB56B, 0x2D0D85FD, 0xB404D447, 0xC303E4D1, 0x5D677172, 0x2A6041E4, 0xB369105E, 0xC46E20C8,
    0x72080DF5, 0x050F3D63, 0x9C066CD9, 0xEB015C4F, 0x7565C9EC, 0x0262F97A, 0x9B6BA8C0, 0xEC6C9856,
    0x7CD385C7, 0x0BD4B551, 0x92DDE4EB, 0xE5DAD47D, 0x7BBE41DE, 0x0CB97148, 0x95B020F2, 0xE2B71064,
    0x6FBF1D91, 0x18B82D07, 0x81B17CBD, 0xF6B64C2B, 0x68D2D988, 0x1FD5E91E, 0x86DCB8A4, 0xF1DB8832,
    0x616495A3, 0x1663A535, 0x8F6AF48F, 0xF86DC419, 0x660951BA, 0x110E612C, 0x88073096, 0xFF000000
};

/* SOURCE: http://home.thep.lu.se/~bjorn/crc/ */
static uint32_t crc32(const uint8_t* data, size_t size)
{
    if (!data || !size)
    {
        return 0;
    }

    uint32_t crc = 0;

    for (size_t i = 0; i < size; ++i)
    {
        crc = CRC32_TABLE[(uint8_t)crc ^ data[i]] ^ crc >> 8;
    }

    return crc;
}

/* this can fail if the int is bigger than 8 bytes */
static size_t vln_read(const uint8_t* data, size_t* offset)
{
    size_t result = 0;
    size_t shift = 0;

    /* just in case its a bad patch, only run until max size */
    for (uint8_t i = 0; i < sizeof(size_t); ++i)
    {
        const uint8_t value = data[*offset];
        ++*offset;

        if (value & 0x80)
        {
            result += (value & 0x7F) << shift;
            break;
        }

        result += (value | 0x80) << shift;
        shift += 7;
    }

    return result;
}

static uint8_t safe_read(const uint8_t* data, size_t* offset, size_t size)
{
    if (*offset < size)
    {
        const uint8_t value = data[*offset];
        ++*offset;

        return value;
    }

    return 0;
}

static void safe_write(uint8_t* data, uint8_t value, size_t* offset, size_t size)
{
    if (*offset < size)
    {
        data[*offset] = value;
        ++*offset;
    }
}

bool ups_verify_header(const uint8_t* patch, size_t patch_size)
{
    if (!patch || patch_size < PATCH_HEADER_SIZE)
    {
        return false;
    }

    /* verify header */
    if (patch[0] != 'U' || patch[1] != 'P' ||
        patch[2] != 'S' || patch[3] != '1'
    ) {
        return false;
    }

    return true;
}

bool ups_get_sizes(
    const uint8_t* patch, size_t patch_size,
    size_t* dst_size, size_t* src_size, size_t* offset
) {
    (void)patch_size; // unused

    /* the offset is after the header */
    size_t offset_local = PATCH_HEADER_SIZE;

    const size_t input_size = vln_read(patch, &offset_local);
    const size_t output_size = vln_read(patch, &offset_local);

    if (src_size != NULL)
    {
        *src_size = input_size;
    }
    if (dst_size != NULL)
    {
        *dst_size = output_size;
    }
    if (offset != NULL)
    {
        *offset = offset_local;
    }

    return true;
}

/* applies the ups patch to the dst data */
bool ups_patch(
    uint8_t* dst, size_t dst_size,
    const uint8_t* src, size_t src_size,
    const uint8_t* patch, size_t patch_size
) {
    if (!dst || !dst_size || !src || !src_size ||
        !patch || patch_size < PATCH_MIN_SIZE)
    {
        return false;
    }

    size_t patch_offset = 0;
    size_t input_size = 0;
    size_t output_size = 0;

    if (ups_verify_header(patch, patch_size))
    {
        return false;
    }

    if (ups_get_sizes(patch, patch_size, &input_size, &output_size, &patch_offset))
    {
        return false;
    }

    if (dst_size < output_size)
    {
        return false;
    }

    /* crc's are at the last 12 bytes, each 4 bytes each. */
    /* memcpy can also work here, but a simple cast works fine. */
    const uint32_t src_crc = *(uint32_t*)(patch + (patch_size - 12));
    const uint32_t dst_crc = *(uint32_t*)(patch + (patch_size - 8));
    const uint32_t patch_crc = *(uint32_t*)(patch + (patch_size - 4));

    /* check that the src and patch is valid. */
    /* dst is checked at the end. */
    const uint32_t src_crc2 = crc32(src, src_size);
    /* we don't check it's own crc32 (obviously) */
    const uint32_t patch_crc2 = crc32(patch, patch_size - 4);

    #define CHECK_CRC(a, b) if (a != b) { return false; }

    CHECK_CRC(patch_crc, patch_crc2);
    CHECK_CRC(src_crc, src_crc2);

    /* we've read the crc's now, reduce the size. */
    patch_size -= 12;

    size_t src_offset = 0;
    size_t dst_offset = 0;

    /* read hunks and patch */
    while (patch_offset < patch_size)
    {
        size_t len = vln_read(patch, &patch_offset);

        while (len-- && dst_offset < dst_size)
        {
            const uint8_t value = safe_read(src, &src_offset, src_size);
            safe_write(dst, value, &dst_offset, dst_size);
        }

        while (dst_offset < dst_size)
        {
            const uint8_t patch_value = safe_read(patch, &patch_offset, patch_size);
            const uint8_t src_value = safe_read(src, &src_offset, src_size);

            safe_write(dst, src_value ^ patch_value, &dst_offset, dst_size);

            if (patch_value == 0)
            {
                break;
            }
        }
    }

    /* patch can be smaller than src, in this case keep writing from src */
    while (src_offset < src_size && dst_offset < dst_size)
    {
        const uint8_t value = safe_read(src, &src_offset, src_size);
        safe_write(dst, value, &dst_offset, dst_size);
    }

    const uint32_t dst_crc2 = crc32(dst, dst_size);
    CHECK_CRC(dst_crc, dst_crc2);

    return true;
}
