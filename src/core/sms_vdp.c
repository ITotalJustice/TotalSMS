#include "sms.h"
#include "sms_internal.h"
#include "sms_types.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>


#define VDP sms->vdp

#ifndef SMS_PIXEL_WIDTH
    typedef uint32_t pixel_width_t;
#else
    #if SMS_PIXEL_WIDTH == 32
        typedef uint32_t pixel_width_t;
    #elif SMS_PIXEL_WIDTH == 16
        typedef uint16_t pixel_width_t;
    #elif SMS_PIXEL_WIDTH == 8
        typedef uint8_t pixel_width_t;
    #else
        #error "invalid SMS_PIXEL_WIDTH! use [8,16,32]"
    #endif
#endif

enum VdpScanlineRenderType {
    VdpScanlineRenderType_8bit = 1,
    VdpScanlineRenderType_16bit = 2,
    VdpScanlineRenderType_32bit = 4,
};

enum
{
    NTSC_HCOUNT_MAX = 342,
    NTSC_VCOUNT_MAX = 262,
    PAL_VCOUNT_MAX = 313,

    NTSC_FPS = 60,
    PAL_FPS = 50,

    NTSC_CYCLES_PER_LINE = 228, // SMS_CPU_CLOCK / NTSC_VCOUNT_MAX / NTSC_FPS, // ~228

    // rounded up
    NTSC_ACTIVE_DISPLAY_CYCLES = 171, // 512 mclks / 3 = ~170
    // rounded down
    NTSC_BLANKING_BORDER_CYCLES = 57, // 172 mclks / 3 = ~57
    // rounded up
    NTSC_FRAME_INTERRUPT_CYCLES = 203, // 607 mclks / 3 = ~202
    // rounded up
    NTSC_LINE_INTERRUPT_CYCLES = 203, // 608 mclks / 3 = ~202
};

enum VdpHeightMode {
    VdpHeightMode_192,
    VdpHeightMode_224,
    VdpHeightMode_240,
};

static const uint16_t VDP_VCOUNT_MAX[2] = {
    [SMS_Region_NTSC] = NTSC_VCOUNT_MAX,
    [SMS_Region_PAL] = PAL_VCOUNT_MAX,
};

// values returned when reading vcount io port for each height mode
static const uint8_t VDP_VCOUNT_PORT[2][3][PAL_VCOUNT_MAX] = {
    [SMS_Region_NTSC] = {
        [VdpHeightMode_192] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, /* wrap around */
            0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
            0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
            0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
        },
        [VdpHeightMode_224] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
            0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, /* wrap around */
            0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
            0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
        },
        [VdpHeightMode_240] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
            0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05
        },
    },
    [SMS_Region_PAL] = {
        [VdpHeightMode_192] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
            0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
            0xF0, 0xF1, 0xF2, /* wrap around */
            0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
            0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
            0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
            0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
            0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
        },
        [VdpHeightMode_224] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
            0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
            0x00, 0x01, 0x02, /* wrap around */
            0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
            0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
            0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9,
            0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
        },
        [VdpHeightMode_240] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
            0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
            0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
            0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
            0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
            0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, /* wrap around */
            0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1,
            0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1,
            0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
        },
    },
};

static const uint8_t VDP_ACTIVE_DISPLAY_HEIGHT[3] = {
    [VdpHeightMode_192] = 192,
    [VdpHeightMode_224] = 224,
    [VdpHeightMode_240] = 240,
};

static const uint8_t NTSC_NEXT_EVENT_CYCLES[2] = {
    [VdpState_ACTIVE] = NTSC_ACTIVE_DISPLAY_CYCLES,
    [VdpState_BLANKING] = NTSC_BLANKING_BORDER_CYCLES,
};

// used to render pixels before converting to frontend native format.
// this buffer isn't used if pixel_width_t matches the frontend bpp.
// in that case, the frontend pixel buffer is used directly.
static pixel_width_t SCANLINE_BUFFER[SMS_SCREEN_WIDTH];

// SOURCE: https://www.smspower.org/forums/8161-SMSDisplayTiming
// (divide mclks by 3)
// 59,736 (179,208 mclks, 228x262)
// frame_int 202 cycles into line 192 (607 mclks)
// line_int 202 cycles into triggering (608 mclks)

static inline bool vdp_is_line_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 4);
}

static inline bool vdp_is_vblank_irq_wanted(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 5);
}

static bool vdp_is_screen_size_change_enabled(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 1);
}

static bool vdp_is_mode4(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x0], 2) && !SMS_is_system_type_sg(sms);
}

static uint16_t vdp_get_nametable_base_addr(const struct SMS_Core* sms)
{
    if (vdp_is_mode4(sms))
    {
        // todo: handle mask bit on sms1
        return (VDP.registers[0x2] & 0xE) << 10;
    }
    else
    {
        return (VDP.registers[0x2] & 0xF) << 10;
    }
}

static uint16_t vdp_get_sprite_attribute_base_addr(const struct SMS_Core* sms)
{
    if (vdp_is_mode4(sms))
    {
        // todo: handle mask bit on sms1
        return (VDP.registers[0x5] & 0x7E) * 128;
    }
    else
    {
        return (VDP.registers[0x5] & 0x7F) * 128;
    }
}

static bool vdp_get_sprite_pattern_select(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x6], 2);
}

static bool vdp_is_display_enabled(const struct SMS_Core* sms)
{
    return IS_BIT_SET(VDP.registers[0x1], 6);
}

static uint8_t vdp_get_sprite_height(const struct SMS_Core* sms)
{
    const bool doubled_sprites = IS_BIT_SET(VDP.registers[0x1], 0);
    const uint8_t sprite_size = IS_BIT_SET(VDP.registers[0x1], 1) ? 16 : 8;

    return sprite_size << doubled_sprites;
}

// returns the height of the screen
static enum VdpHeightMode vdp_get_height_mode(const struct SMS_Core* sms)
{
    if (vdp_is_mode4(sms) && vdp_is_screen_size_change_enabled(sms))
    {
        if (IS_BIT_SET(VDP.registers[1], 4))
        {
            assert(!"screen mode not impelented!");
            return VdpHeightMode_224;
        }
        else if (IS_BIT_SET(VDP.registers[1], 3))
        {
            assert(!"screen mode not impelented!");
            return VdpHeightMode_240;
        }
    }

    return VdpHeightMode_192;
}

static uint8_t vdp_get_overscan_colour(const struct SMS_Core* sms)
{
    return VDP.registers[0x7] & 0xF;
}

static uint32_t vdp_get_overscan_colour_converted(const struct SMS_Core* sms)
{
    if (vdp_is_mode4(sms))
    {
        return VDP.colour[16 + vdp_get_overscan_colour(sms)];
    }
    else
    {
        return sms->builtin_palette[vdp_get_overscan_colour(sms)];
    }
}

struct VDP_region
{
    uint16_t startx;
    uint16_t endx;
    uint16_t pixelx;
    uint16_t pixely;
};

static struct VDP_region vdp_get_region(const struct SMS_Core* sms)
{
    if (SMS_is_system_type_gg(sms))
    {
        return (struct VDP_region)
        {
            .startx = 48,
            .endx = 160 + 48,
            .pixelx = 62 - 48,
            .pixely = VDP.vcount + 27,
        };
    }
    else
    {
        return (struct VDP_region)
        {
            .startx = IS_BIT_SET(VDP.registers[0x0], 5) ? 8 : 0,
            .endx = SMS_SCREEN_WIDTH,
            .pixelx = 0,
            .pixely = VDP.vcount,
        };
    }
}

static bool vdp_is_display_active_vcount(const struct SMS_Core* sms, uint16_t vcount)
{
    if (SMS_is_system_type_gg(sms))
    {
        return vcount >= 24 && vcount < 144 + 24;
    }
    else
    {
        return vcount < VDP_ACTIVE_DISPLAY_HEIGHT[vdp_get_height_mode(sms)];
    }
}

static bool vdp_is_display_active(const struct SMS_Core* sms)
{
    return vdp_is_display_active_vcount(sms, VDP.vcount);
}

static enum VdpScanlineRenderType get_scanline_type(const struct SMS_Core* sms)
{
    switch (sms->bpp)
    {
        case 1:
        case 8:
            return VdpScanlineRenderType_8bit;

        case 2:
        case 15:
        case 16:
            return VdpScanlineRenderType_16bit;

        case 4:
        case 24:
        case 32:
            return VdpScanlineRenderType_32bit;
    }

    return -1;
}

#ifndef SMS_PIXEL_WIDTH
static void write_scanline_to_frame(struct SMS_Core* sms, const pixel_width_t* scanline, const uint8_t y)
{
    switch (get_scanline_type(sms))
    {
        case VdpScanlineRenderType_8bit: {
            uint8_t* pixels = &((uint8_t*)sms->pixels)[sms->stride * y];
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint8_t)scanline[i];
            }
        }   break;

        case VdpScanlineRenderType_16bit: {
            uint16_t* pixels = ((uint16_t*)sms->pixels) + (sms->stride * y);
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint16_t)scanline[i];
            }
        }   break;

        case VdpScanlineRenderType_32bit: {
            uint32_t* pixels = &((uint32_t*)sms->pixels)[sms->stride * y];
            for (int i = 0; i < SMS_SCREEN_WIDTH; ++i)
            {
                pixels[i] = (uint32_t)scanline[i];
            }
        }   break;

        default:
            assert(!"bpp invalid option!");
            break;
    }
}
#endif

uint8_t vdp_io_read_vcounter(const struct SMS_Core* sms)
{
    return VDP_VCOUNT_PORT[sms->region][vdp_get_height_mode(sms)][sms->vdp.vcount];
}

uint8_t vdp_io_read_hcounter(const struct SMS_Core* sms)
{
    const int event_cycles = scheduler_get_event_cycles(&sms->scheduler, SchedulerID_VDP);
    int elsaped = 0;

    switch (VDP.state)
    {
        case VdpState_ACTIVE:
            elsaped = NTSC_ACTIVE_DISPLAY_CYCLES - event_cycles;
            break;

        case VdpState_BLANKING:
            elsaped = NTSC_ACTIVE_DISPLAY_CYCLES + NTSC_BLANKING_BORDER_CYCLES - event_cycles;
            break;
    }

    // assert(!"reading hcounter is properly unimplemented...");
    // the difference between is how many cycles elapsed since start of line.
    uint16_t value = elsaped;
    // ppu is 1.5x faster than the cpu.
    value = (float)value * 1.5F;
    // docs say that this is a 9-bit counter, but only upper 8-bits read.
    return value >> 1;
}

uint8_t vdp_io_data_read(struct SMS_Core* sms)
{
    sms->vdp.control_latch = false;

    const uint8_t data = sms->vdp.buffer_read_data;

    sms->vdp.buffer_read_data = sms->vdp.vram[sms->vdp.addr];
    sms->vdp.addr = (sms->vdp.addr + 1) & 0x3FFF;

    return data;
}

uint8_t vdp_io_status_read(struct SMS_Core* sms)
{
    sms->vdp.control_latch = false;

    uint8_t v = 0;
    v |= VDP.frame_interrupt_pending << 7;
    v |= VDP.sprite_overflow << 6;
    v |= VDP.sprite_collision << 5;

    // these are reset on read
    VDP.frame_interrupt_pending = false;
    VDP.line_interrupt_pending = false;
    VDP.sprite_overflow = false;
    VDP.sprite_collision = false;

    if (vdp_is_mode4(sms))
    {
        v |= 31; // unused bits 1,2,3,4
    }
    else
    {
        v |= VDP.fifth_sprite_num;
        VDP.fifth_sprite_num = 0;
    }

    return v;
}

static void IO_vdp_cram_gg_write(struct SMS_Core* sms, const uint8_t value)
{
    // even addr stores byte to latch, odd writes 2 bytes
    if (sms->vdp.addr & 1)
    {
        const uint8_t rg_index = (sms->vdp.addr - 1) & 0x3F;
        const uint8_t b_index = sms->vdp.addr & 0x3F;

        // check is the colour has changed, if so, set dirty
        if (sms->vdp.cram[rg_index] != sms->vdp.cram_gg_latch || sms->vdp.cram[b_index] != value)
        {
            sms->vdp.dirty_cram_min = SMS_MIN(sms->vdp.dirty_cram_min, rg_index);
            sms->vdp.dirty_cram_max = SMS_MAX(sms->vdp.dirty_cram_max, rg_index+1);
            sms->vdp.dirty_cram[rg_index] = true;
        }

        sms->vdp.cram[rg_index] = sms->vdp.cram_gg_latch;
        sms->vdp.cram[b_index] = value;
    }
    else
    {
        // latches the r,g values
        sms->vdp.cram_gg_latch = value;
    }
}

static void IO_vdp_cram_sms_write(struct SMS_Core* sms, const uint8_t value)
{
    const uint8_t index = sms->vdp.addr & 0x1F;

    if (sms->vdp.cram[index] != value)
    {
        sms->vdp.cram[index] = value;
        sms->vdp.dirty_cram[index] = true;
        sms->vdp.dirty_cram_min = SMS_MIN(sms->vdp.dirty_cram_min, index);
        sms->vdp.dirty_cram_max = SMS_MAX(sms->vdp.dirty_cram_max, index+1);
    }
}

static void vdp_io_write(struct SMS_Core* sms, const uint8_t addr, const uint8_t value)
{
    if (SMS_is_system_type_sg(sms))
    {
        VDP.registers[addr & 0x7] = value;
    }
    else
    {
        switch (addr & 0xF)
        {
            case 0x0:
                // assert(IS_BIT_SET(value, 2) && "not mode4, using TMS9918 modes!");
                // assert(IS_BIT_SET(value, 1) && !IS_BIT_SET(VDP.registers[1], 3) && "240 height mode set");
                // assert(IS_BIT_SET(value, 1) && !IS_BIT_SET(VDP.registers[1], 4) && "224 height mode set");
                break;

            case 0x1:
                // assert(!IS_BIT_SET(value, 3) && IS_BIT_SET(VDP.registers[0], 1) && "240 height mode set");
                // assert(!IS_BIT_SET(value, 4) && IS_BIT_SET(VDP.registers[0], 1) && "224 height mode set");
                break;

            case 0x3:
                if (vdp_is_mode4(sms) && value != 0xFF)
                {
                    assert(value == 0xFF && "colour table bits not all set");
                }
                break;

            case 0x4:
                if (vdp_is_mode4(sms) && (value & 0x7) != 0x7)
                {
                    // assert((value & 0x7) == 0x7 && "low 3 bits should be set in m4");
                }
                break;

            // unused registers
            case 0xB:
            case 0xC:
            case 0xD:
            case 0xE:
            case 0xF:
                return;
        }

        VDP.registers[addr & 0xF] = value;
    }

    z80_check_for_irq(sms);
}

void vdp_io_data_write(struct SMS_Core* sms, const uint8_t value)
{
    sms->vdp.control_latch = false;
    // writes store the new value in the buffered_data
    sms->vdp.buffer_read_data = value;

    switch (sms->vdp.code)
    {
        case VDP_CODE_VRAM_WRITE_LOAD:
        case VDP_CODE_VRAM_WRITE:
        case VDP_CODE_REG_WRITE:
            // mark entry as dirty if modified
            sms->vdp.dirty_vram[sms->vdp.addr >> 2] |= sms->vdp.vram[sms->vdp.addr] != value;
            sms->vdp.vram[sms->vdp.addr] = value;
            sms->vdp.addr = (sms->vdp.addr + 1) & 0x3FFF;
            break;

        case VDP_CODE_CRAM_WRITE:
            switch (SMS_get_system_type(sms))
            {
                case SMS_System_SMS:
                    IO_vdp_cram_sms_write(sms, value);
                    break;

                case SMS_System_GG:
                    IO_vdp_cram_gg_write(sms, value);
                    break;

                case SMS_System_SG1000:
                    assert(!"sg1000 cram write, check what should happen here!");
                    break;
            }

            sms->vdp.addr = (sms->vdp.addr + 1) & 0x3FFF;
            break;
    }
}

void vdp_io_control_write(struct SMS_Core* sms, const uint8_t value)
{
    if (sms->vdp.control_latch)
    {
        sms->vdp.control_word = (sms->vdp.control_word & 0xFF) | (value << 8);
        sms->vdp.code = (value >> 6) & 3;
        sms->vdp.control_latch = false;

        switch (sms->vdp.code)
        {
            // code0 immediatley loads a byte from vram into buffer
            case VDP_CODE_VRAM_WRITE_LOAD:
                sms->vdp.addr = sms->vdp.control_word & 0x3FFF;
                sms->vdp.buffer_read_data = sms->vdp.vram[sms->vdp.addr];
                sms->vdp.addr = (sms->vdp.addr + 1) & 0x3FFF;
                break;

            case VDP_CODE_VRAM_WRITE:
                sms->vdp.addr = sms->vdp.control_word & 0x3FFF;
                break;

            case VDP_CODE_REG_WRITE:
                vdp_io_write(sms, value & 0xF, sms->vdp.control_word & 0xFF);
                break;

            case VDP_CODE_CRAM_WRITE:
                sms->vdp.addr = sms->vdp.control_word & 0x3FFF;
                break;
        }
    }
    else
    {
        sms->vdp.addr = (sms->vdp.addr & 0x3F00) | value;
        sms->vdp.control_word = value;
        sms->vdp.control_latch = true;
    }
}

// same as i used in dmg / gbc rendering for gb
struct PriorityBuf
{
    bool array[SMS_SCREEN_WIDTH];
};

static void vdp_mode2_render_background(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;
    const uint8_t overscan_colour = vdp_get_overscan_colour(sms);

    const uint16_t pattern_table_addr = (VDP.registers[4] & 0x04) << 11;
    const uint16_t colour_map_addr = (VDP.registers[3] & 0x80) << 6;
    const uint16_t region = (VDP.registers[4] & 0x03) << 8;

    for (uint8_t col = 0; col < 32; col++)
    {
        const uint16_t tile_number = (row * 32) + col;
        const uint16_t name_tile_addr = vdp_get_nametable_base_addr(sms) + tile_number;
        const uint16_t name_tile = VDP.vram[name_tile_addr] | (region & 0x300 & tile_number);

        const uint8_t pattern_line = VDP.vram[pattern_table_addr + (name_tile * 8) + fine_line];
        const uint8_t color_line = VDP.vram[colour_map_addr + (name_tile * 8) + fine_line];

        const uint8_t bg_color = color_line & 0x0F;
        const uint8_t fg_color = color_line >> 4;

        for (uint8_t x = 0; x < 8; x++)
        {
            const uint8_t x_index = (col * 8) + x;

            uint8_t colour = IS_BIT_SET(pattern_line, 7 - x) ? fg_color : bg_color;
            colour = (colour > 0) ? colour : overscan_colour;
            scanline[x_index] = sms->builtin_palette[colour];
        }
    }
}

static void vdp_mode1_render_background(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;
    const uint8_t overscan_colour = vdp_get_overscan_colour(sms);

    const uint16_t name_table_addr = (VDP.registers[2] & 0x0F) << 10;
    const uint16_t pattern_table_addr = (VDP.registers[4] & 0x07) << 11;
    const uint16_t colour_map_addr = VDP.registers[3] << 6;

    for (uint8_t col = 0; col < 32; col++)
    {
        const uint16_t tile_number = (row * 32) + col;
        const uint16_t name_tile_addr = name_table_addr + tile_number;
        const uint16_t name_tile = VDP.vram[name_tile_addr];

        const uint8_t pattern_line = VDP.vram[pattern_table_addr + (name_tile * 8) + fine_line];
        const uint8_t color_line = VDP.vram[colour_map_addr + (name_tile >> 3)];

        const uint8_t bg_color = color_line & 0x0F;
        const uint8_t fg_color = color_line >> 4;

        for (uint8_t x = 0; x < 8; x++)
        {
            const uint8_t x_index = (col * 8) + x;

            uint8_t colour = IS_BIT_SET(pattern_line, 7 - x) ? fg_color : bg_color;
            colour = (colour > 0) ? colour : overscan_colour;
            scanline[x_index] = sms->builtin_palette[colour];
        }
    }
}

static inline struct CachedPalette vdp_get_palette(struct SMS_Core* sms, const uint16_t pattern_index)
{
    struct CachedPalette* cpal = &VDP.cached_palette[pattern_index >> 2];

    // check if one of the 4 bit_planes have changed
    if (VDP.dirty_vram[pattern_index >> 2])
    {
        VDP.dirty_vram[pattern_index >> 2] = false;

        const uint8_t bit_plane0 = VDP.vram[pattern_index + 0];
        const uint8_t bit_plane1 = VDP.vram[pattern_index + 1];
        const uint8_t bit_plane2 = VDP.vram[pattern_index + 2];
        const uint8_t bit_plane3 = VDP.vram[pattern_index + 3];

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t bit_flip = x;
            const uint8_t bit_norm = 7 - x;

            cpal->flipped <<= 4;
            cpal->flipped |= IS_BIT_SET(bit_plane0, bit_flip) << 0;
            cpal->flipped |= IS_BIT_SET(bit_plane1, bit_flip) << 1;
            cpal->flipped |= IS_BIT_SET(bit_plane2, bit_flip) << 2;
            cpal->flipped |= IS_BIT_SET(bit_plane3, bit_flip) << 3;

            cpal->normal <<= 4;
            cpal->normal |= IS_BIT_SET(bit_plane0, bit_norm) << 0;
            cpal->normal |= IS_BIT_SET(bit_plane1, bit_norm) << 1;
            cpal->normal |= IS_BIT_SET(bit_plane2, bit_norm) << 2;
            cpal->normal |= IS_BIT_SET(bit_plane3, bit_norm) << 3;
        }
    }

    return *cpal;
}

static void vdp_render_background(struct SMS_Core* sms, pixel_width_t* scanline, struct PriorityBuf* prio)
{
    const struct VDP_region region = vdp_get_region(sms);

    const uint8_t line = VDP.vcount;
    const uint8_t fine_line = line & 0x7;
    const uint8_t row = line >> 3;

    // check if horizontal scrolling should be disabled
    // doesn't work in GG mode as the screen centered
    const bool horizontal_disabled = IS_BIT_SET(VDP.registers[0x0], 6) && line < 16;

    const uint8_t starting_col = (32 - (VDP.registers[0x8] >> 3)) & 31;
    const uint8_t fine_scrollx = horizontal_disabled ? 0 : VDP.registers[0x8] & 0x7;
    const uint8_t horizontal_scroll = horizontal_disabled ? 0 : starting_col;

    const uint8_t starting_row = VDP.vertical_scroll >> 3;
    const uint8_t fine_scrolly = VDP.vertical_scroll & 0x7;

    const uint8_t* nametable = NULL;
    uint8_t check_col = 0;
    uint8_t palette_index_offset = 0;

    // if set, we use the internal col counter, else, the starting_col
    if (!IS_BIT_SET(VDP.registers[0x1], 7))
    {
        check_col = (starting_col) & 31;
    }

    {
        // we need to check if we cross the next row
        const bool next_row = (fine_line + fine_scrolly) > 7;

        const uint16_t vertical_offset = ((row + starting_row + next_row) % 28) * 64;
        palette_index_offset = (fine_line + fine_scrolly) & 0x7;
        nametable = &VDP.vram[vdp_get_nametable_base_addr(sms) + vertical_offset];
    }

    if (region.startx == 8)
    {
        // render overscan
        const uint8_t palette_index = 16 + vdp_get_overscan_colour(sms);

        for (int x_index = 0; x_index < 8; x_index++)
        {
            // used when sprite rendering, will skip if prio set and not pal0
            prio->array[x_index] = true;//priority && palette_index != 0;
            scanline[x_index] = VDP.colour[palette_index];
        }
    }

    for (uint8_t col = 0; col < 32; ++col)
    {
        check_col = (check_col + 1) & 31;
        const uint16_t horizontal_offset = ((horizontal_scroll + col) & 31) * 2;

        // check if vertical scrolling should be disabled
        if (IS_BIT_SET(VDP.registers[0x0], 7) && check_col >= 24)
        {
            const uint16_t vertical_offset = (row % 28) * 64;
            palette_index_offset = fine_line;
            nametable = &VDP.vram[vdp_get_nametable_base_addr(sms) + vertical_offset];
        }

        const uint16_t tile = mem_read16(nametable + horizontal_offset);

        // if set, background will display over sprites
        const bool priority = IS_BIT_SET(tile, 12);
        // select from either sprite or background palette
        const bool palette_select = IS_BIT_SET(tile, 11);
        // vertical flip
        const bool vertical_flip = IS_BIT_SET(tile, 10);
        // horizontal flip
        const bool horizontal_flip = IS_BIT_SET(tile, 9);
        // one of the 512 patterns to select
        uint16_t pattern_index = (tile & 0x1FF) * 32;

        if (vertical_flip)
        {
            pattern_index += (7 - palette_index_offset) * 4;
        }
        else
        {
            pattern_index += palette_index_offset * 4;
        }

        const struct CachedPalette cpal = vdp_get_palette(sms, pattern_index);
        const uint32_t palette = horizontal_flip ? cpal.flipped : cpal.normal;
        const uint8_t pal_base = palette_select ? 16 : 0;

        for (uint8_t x = 0; x < 8; ++x)
        {
            const uint8_t x_index = ((col * 8) + x + fine_scrollx) % SMS_SCREEN_WIDTH;

            if (x_index >= region.endx)
            {
                break;
            }

            if (x_index < region.startx)
            {
                continue;
            }

            const uint8_t palette_index =  (palette >> (28 - (4 * x))) & 0xF;
            prio->array[x_index] = priority && palette_index != 0;
            scanline[x_index] = VDP.colour[pal_base + palette_index];
        }
    }
}

void SMS_get_pixel_region(const struct SMS_Core* sms, int* x, int* y, int* w, int* h)
{
    // todo: support different height modes
    if (SMS_is_system_type_gg(sms))
    {
        *x = 48;
        *y = 24;
        *w = 160;
        *h = 144;
    }
    else
    {
        const enum VdpHeightMode hieght_mode = vdp_get_height_mode(sms);

        *x = 0;
        *y = (SMS_SCREEN_HEIGHT - VDP_ACTIVE_DISPLAY_HEIGHT[hieght_mode]) / 2;
        *w = SMS_SCREEN_WIDTH;
        *h = VDP_ACTIVE_DISPLAY_HEIGHT[hieght_mode];
    }
}

static void vdp_parse_sg_sprites(struct SMS_Core* sms, int line)
{
    memset(VDP.sprites, 0, sizeof(VDP.sprites));
    VDP.sprites_count = 0;

    if (!SMS_is_system_type_sg(sms))
    {
        // assert(IS_BIT_SET(VDP.registers[0x5], 0) && "needs lower index for oam");
        // assert((VDP.registers[0x6] & 0x3) == 0x3 && "Sprite Pattern Generator Base Address");
    }

    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
    const uint8_t sprite_size = vdp_get_sprite_height(sms);
    // https://konamiman.github.io/MSX2-Technical-Handbook/md/Chapter4a.html#sprite-attribute-table
    const int sprite_eof = 208; // 216 in mode 2

    for (uint8_t i = 0; i < 128; i += 4)
    {
        // todo: find out how the sprite y wrapping works!
        int16_t y = VDP.vram[sprite_attribute_base_addr + i + 0] + 1;

        // special number used to stop sprite parsing!
        if (y == sprite_eof + 1)
        {
            break;
        }

        // todo: is this based on the screen height
        // if (y > 192)
        if (y > 224)
        {
            y -= 256;
        }

        if (line >= y && line < (y + sprite_size))
        {
            const int16_t x = VDP.vram[sprite_attribute_base_addr + i + 1];
            const uint8_t tile_num = VDP.vram[sprite_attribute_base_addr + i + 2];
            const uint8_t colour = VDP.vram[sprite_attribute_base_addr + i + 3];

            struct VdpSpriteEntry* sprite = &VDP.sprites[VDP.sprites_count];
            sprite->y = y;
            // note: docs say its shifted 32 pixels NOT 8!
            sprite->x = x - (IS_BIT_SET(colour, 7) ? 32 : 0);
            sprite->tile_num = sprite_size > 8 ? tile_num & ~0x3 : tile_num;
            sprite->colour = colour & 0xF;
            VDP.sprites_count++;
        }
    }
}

static void vdp_mode1_render_sprites(struct SMS_Core* sms, pixel_width_t* scanline)
{
    const uint8_t line = VDP.vcount;
    const uint16_t tile_addr = (VDP.registers[0x6] & 0x7) * 0x800;
    const uint8_t sprite_size = vdp_get_sprite_height(sms);
    // const struct SgSpriteEntries sprites = vdp_parse_sg_sprites(sms);
    bool drawn_sprites[SMS_SCREEN_WIDTH] = {0};
    uint8_t sprite_rendered_count = 0;
    const uint8_t max_sprites = sms->mode1_max_spirtes;

    for (uint8_t i = 0; i < VDP.sprites_count; i++)
    {
        const struct VdpSpriteEntry* sprite = &VDP.sprites[i];

        if (sprite->colour == 0)
        {
            continue;
        }

        uint8_t pattern_line = VDP.vram[tile_addr + (line - sprite->y) + (sprite->tile_num * 8)];
        // check if we actually drawn a sprite
        bool did_draw_a_sprite = false;

        for (uint8_t x = 0; x < sprite_size; x++)
        {
            const int16_t x_index = x + sprite->x;

            // skip if column0 or offscreen
            if (x_index < 0)
            {
                continue;
            }

            if (x_index >= SMS_SCREEN_WIDTH)
            {
                break;
            }

            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // idk what name to give this
            // basically, a sprite can be 32x32 but it doubles
            // up on the bits so that bit0 will be for pixel 0,1
            const uint8_t x2 = x >> (sprite_size == 32);

            if (x2 == 8)
            {
                // reload
                pattern_line = VDP.vram[tile_addr + (line - sprite->y) + (sprite->tile_num * 8) + 16];
            }

            if (!IS_BIT_SET(pattern_line, 7 - (x2 & 7)))
            {
                continue;
            }

            if (sprite_rendered_count < 4) // max of 4 sprites lol
            {
                did_draw_a_sprite = true;
            }

            if (sprite_rendered_count < max_sprites)
            {
                drawn_sprites[x_index] = true;
                scanline[x_index] = sms->builtin_palette[sprite->colour];
            }
        }

        // update count if we drawn a sprite
        sprite_rendered_count += did_draw_a_sprite;
        // now check if theres more than 5 spirtes on a line
        if (i >= 4)
        {
            VDP.sprite_overflow = true;
            VDP.fifth_sprite_num = i;
        }
    }
}

// todo: spirtes are pasrsed 1 line ahead!
static void vdp_mode4_parse_sprites(struct SMS_Core* sms, int line)
{
    assert(vdp_is_mode4(sms) && "called the wrong function 4head");
    assert(!SMS_is_system_type_sg(sms) && "read above message");

    memset(VDP.sprites, 0, sizeof(VDP.sprites));
    VDP.sprites_count = 0;

    const uint16_t sprite_attribute_base_addr = vdp_get_sprite_attribute_base_addr(sms);
    const uint8_t sprite_attribute_x_index = IS_BIT_SET(VDP.registers[0x5], 0) ? 128 : 0;
    const uint8_t sprite_size = vdp_get_sprite_height(sms);
    const uint8_t max_sprites = sms->mode4_max_spirtes;

    const int sprite_eof = 208;

    for (uint8_t i = 0; i < 64; ++i)
    {
        // todo: find out how the sprite y wrapping works!
        int16_t y = VDP.vram[sprite_attribute_base_addr + i] + 1;
        // int16_t y = VDP.vram[sprite_attribute_base_addr + i];// + 1;

        // special number used to stop sprite parsing!
        // todo:
        if (y == sprite_eof + 1)
        // if (y == sprite_eof)
        {
            break;
        }

        // docs (TMS9918.pdf) state that y is partially signed (-31, 0) which
        // is line 224 im not sure if this is correct, as it likely isn't as
        // theres a 240 height mode, meaning no sprites could be displayed past 224...
        // so maybe this should reset on 240? or maybe it depends of the
        // the height mode selected, ie, 192, 224 and 240
        #if 1
        if (y > 224)
        #else
        if (y > 192)
        // if (y > 240) // untested
        #endif
        {
            y -= 256;
        }

        // todo: offscreen sprites can still collide
        if (line >= y && line < (y + sprite_size))
        {
            // if we have filled the sprite array, we need to keep checking further
            // entries just in case another sprite falls on the same line, in which
            // case, the sprite overflow flag is set for stat.
            if (VDP.sprites_count >= 8)
            {
                VDP.sprite_overflow = true;
            }

            if (VDP.sprites_count < max_sprites)
            {
                struct VdpSpriteEntry* sprite = &VDP.sprites[VDP.sprites_count];
                sprite->y = y;
                // xn values are either low (0) or high (128) end of SAT
                sprite->x = sprite_attribute_x_index + (i * 2);
                VDP.sprites_count++;
            }
            else
            {
                break;
            }
        }
    }
}

static void vdp_render_sprites(struct SMS_Core* sms, pixel_width_t* scanline, const struct PriorityBuf* prio)
{
    // vdp_mode4_parse_sprites(sms);
    const struct VDP_region region = vdp_get_region(sms);

    const uint8_t line = VDP.vcount;
    const uint16_t attr_addr = vdp_get_sprite_attribute_base_addr(sms);
    // if set, we fetch patterns from upper table
    const uint16_t pattern_select = vdp_get_sprite_pattern_select(sms) ? 256 : 0;
    // if set, sprites start 8 to the left
    const int8_t sprite_x_offset = IS_BIT_SET(VDP.registers[0x0], 3) ? -8 : 0;

    // const struct SpriteEntries sprites = vdp_mode4_parse_sprites(sms);

    bool drawn_sprites[SMS_SCREEN_WIDTH] = {0};

    for (uint8_t i = 0; i < VDP.sprites_count; ++i)
    {
        const struct VdpSpriteEntry* sprite = &VDP.sprites[i];

        // signed because the sprite can be negative if -8!
        const int16_t sprite_x = VDP.vram[attr_addr + sprite->x + 0] + sprite_x_offset;

        if (sprite_x+8 < region.startx || sprite_x>=region.endx)
        {
            continue;
        }

        uint16_t pattern_index = VDP.vram[attr_addr + sprite->x + 1] + pattern_select;

        // docs state that if bit1 of reg1 is set (should always), bit0 is ignored
        // i am not sure if this is applied to the final value of the value
        // initial value however...
        if (IS_BIT_SET(VDP.registers[0x1], 1))
        {
            pattern_index &= ~0x1;
        }

        pattern_index *= 32;

        // this has already taken into account of sprite size when parsing
        // sprites, so for example:
        // - line = 3,
        // - sprite->y = 1,
        // - sprite_size = 8,
        // then y will be accepted for this line, but needs to be offset from
        // the current line we are on, so line-sprite->y = 2
        pattern_index += (line - sprite->y) * 4;

        const struct CachedPalette cpal = vdp_get_palette(sms, pattern_index);
        const uint32_t palette = cpal.normal;//horizontal_flip ? cpal.flipped : cpal.normal;

        // note: the order of the below ifs are important.
        // opaque sprites can collide, even when behind background/
        for (uint8_t x = 0; x < 8; ++x)
        {
            const int16_t x_index = x + sprite_x;

            // skip if column0 or offscreen
            if (x_index < region.startx)
            {
                continue;
            }

            if (x_index >= region.endx)
            {
                break;
            }

            const uint8_t palette_index = (palette >> (28 - (4 * x))) & 0xF;

            // for sprites, pal0 is transparent
            if (palette_index == 0)
            {
                continue;
            }

            // skip if we already rendered a sprite!
            if (drawn_sprites[x_index])
            {
                VDP.sprite_collision = true;
                continue;
            }

            // keep track of this sprite already being rendered
            drawn_sprites[x_index] = true;

            // skip is bg has priority
            if (prio->array[x_index])
            {
                continue;
            }

            // sprite cram index is the upper 16-bytes!
            scanline[x_index] = VDP.colour[palette_index + 16];
        }
    }
}

static void vdp_update_sms_colours(struct SMS_Core* sms)
{
    assert(sms->vdp.dirty_cram_max <= 32);

    for (int i = sms->vdp.dirty_cram_min; i < sms->vdp.dirty_cram_max; i++)
    {
        if (sms->vdp.dirty_cram[i])
        {
            const uint8_t r = (sms->vdp.cram[i] >> 0) & 0x3;
            const uint8_t g = (sms->vdp.cram[i] >> 2) & 0x3;
            const uint8_t b = (sms->vdp.cram[i] >> 4) & 0x3;

            sms->vdp.colour[i] = sms->colour_callback(sms->userdata, r, g, b);
            sms->vdp.dirty_cram[i] = false;
        }
    }

    sms->vdp.dirty_cram_min = sms->vdp.dirty_cram_max = 0;
}

static void vdp_update_gg_colours(struct SMS_Core* sms)
{
    for (int i = sms->vdp.dirty_cram_min; i < sms->vdp.dirty_cram_max; i += 2)
    {
        if (sms->vdp.dirty_cram[i])
        {
            // GG colours are in [----BBBBGGGGRRRR] format
            const uint8_t r = (sms->vdp.cram[i + 0] >> 0) & 0xF;
            const uint8_t g = (sms->vdp.cram[i + 0] >> 4) & 0xF;
            const uint8_t b = (sms->vdp.cram[i + 1] >> 0) & 0xF;

            // only 32 colours, 2 bytes per colour!
            sms->vdp.colour[i >> 1] = sms->colour_callback(sms->userdata, r, g, b);
            sms->vdp.dirty_cram[i] = false;
        }
    }

    sms->vdp.dirty_cram_min = sms->vdp.dirty_cram_max = 0;
}

static void vdp_update_palette(struct SMS_Core* sms)
{
    if (sms->colour_callback && vdp_is_mode4(sms))
    {
        if (SMS_is_system_type_gg(sms))
        {
            vdp_update_gg_colours(sms);
        }
        else
        {
            vdp_update_sms_colours(sms);
        }
    }
}

void vdp_mark_palette_dirty(struct SMS_Core* sms)
{
    memset(sms->vdp.dirty_cram, true, sizeof(sms->vdp.dirty_cram));
    memset(sms->vdp.dirty_vram, true, sizeof(sms->vdp.dirty_vram));
    sms->vdp.dirty_cram_min = 0;

    if (SMS_is_system_type_gg(sms))
    {
        sms->vdp.dirty_cram_max = 64;
    }
    else
    {
        sms->vdp.dirty_cram_max = 32;
    }

    vdp_update_palette(sms);
}

bool vdp_has_interrupt(const struct SMS_Core* sms)
{
    const bool frame_interrupt = VDP.frame_interrupt_pending && vdp_is_vblank_irq_wanted(sms);
    const bool line_interrupt = VDP.line_interrupt_pending && vdp_is_line_irq_wanted(sms);

    return frame_interrupt || line_interrupt;
}

static void vdp_render_line(struct SMS_Core* sms)
{
    // exit early if we have no pixels (this will break games that need sprite overflow and collision)
    // todo: still render spirtes as far as detection even with
    // the screen disabled!
    if (!sms->pixels || sms->skip_frame)
    {
        return;
    }

    // check if vcount is in bounds.
    if (!vdp_is_display_active(sms))
    {
        return;
    }

    struct PriorityBuf prio = {0};
    #ifndef SMS_PIXEL_WIDTH
        const bool is_native_width = get_scanline_type(sms) == sizeof(pixel_width_t);
        pixel_width_t* scanline = SCANLINE_BUFFER;
        if (is_native_width)
        {
            scanline = (pixel_width_t*)sms->pixels + (VDP.vcount * sms->stride);
        }
    #else
        pixel_width_t* scanline = (pixel_width_t*)sms->pixels + (VDP.vcount * sms->stride);
    #endif

    // only render if display is enabled.
    if (vdp_is_display_enabled(sms))
    {
        vdp_update_palette(sms);

        const bool m1 = IS_BIT_SET(VDP.registers[1], 4);
        const bool m2 = IS_BIT_SET(VDP.registers[0], 1);
        const bool m3 = IS_BIT_SET(VDP.registers[1], 3);
        const bool m4 = IS_BIT_SET(VDP.registers[0], 2) && !SMS_is_system_type_sg(sms);

        if (!m4 && !m3 && !m2 && !m1) // Graphic I
        {
            vdp_mode1_render_background(sms, scanline);
            vdp_mode1_render_sprites(sms, scanline);
        }
        else if (!m4 && !m3 && !m2 && m1) // Text
        {
        }
        else if (!m4 && !m3 && m2 && !m1) // Graphic II
        {
            vdp_mode2_render_background(sms, scanline);
            vdp_mode1_render_sprites(sms, scanline);
        }
        else if (!m4 && !m3 && m2 && m1) // Mode 1+2
        {
        }
        else if (!m4 && m3 && !m2 && !m1) // Mulicolor
        {
        }
        else if (!m4 && m3 && !m2 && m1) // Mode 1+3
        {
        }
        else if (!m4 && m3 && m2 && !m1) // Mode 2+3
        {
        }
        else if (!m4 && m3 && m2 && m1) // Mode 1+2+3
        {
        }
        else if (m4 && !m3 && !m2 && !m1) // Mode 4
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
        else if (m4 && !m3 && !m2 && m1) // Invalid text mode
        {
        }
        else if (m4 && !m3 && m2 && !m1) // Mode 4
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
        else if (m4 && !m3 && m2 && m1) // Mode 4 (224-line display)
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
        else if (m4 && m3 && !m2 && !m1) // Mode 4
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
        else if (m4 && m3 && !m2 && m1) // Invalid text mode
        {
        }
        else if (m4 && m3 && m2 && !m1) // Mode 4 (240-line display)
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
        else if (m4 && m3 && m2 && m1) // Mode 4
        {
            vdp_render_background(sms, scanline, &prio);
            vdp_render_sprites(sms, scanline, &prio);
        }
    }
    else
    {
        // screen is blanked, memset the buffer.
        // todo: should this be the overscan colour?
        #if 0
        const uint32_t colour = sms->colour_callback(sms->userdata, 0, 0, 0);
        #else
        vdp_update_palette(sms);
        const uint32_t colour = vdp_get_overscan_colour(sms);
        #endif

        for (int i = 0; i < SMS_SCREEN_WIDTH; i++)
        {
            scanline[i] = colour;
        }
    }

    #ifndef SMS_PIXEL_WIDTH
        if (!is_native_width)
        {
            write_scanline_to_frame(sms, scanline, VDP.vcount);
        }
    #endif
}

// draws a scanline, if in range.
static void on_active_event(struct SMS_Core* sms)
{
    VDP.state = VdpState_BLANKING;
    vdp_render_line(sms);

    // calculate the next line
    const int line = (VDP.vcount + 1) % VDP_VCOUNT_MAX[sms->region];

    // sprite parsing happens 1 line ahead :)
    if (vdp_is_display_active_vcount(sms, line))
    {
        // on sms/gg, sprite overflow still happens with display disabled
        if (vdp_is_mode4(sms))
        {
            vdp_mode4_parse_sprites(sms, line);
        }
        // todo: are sprites still parsed in other modes when enabled?
        else if (vdp_is_display_enabled(sms))
        {
            vdp_parse_sg_sprites(sms, line);
        }
    }
}

// called at the end of a scanline, advances vcount
static void on_blanking_event(struct SMS_Core* sms)
{
    VDP.state = VdpState_ACTIVE;
    VDP.vcount++;

    const enum VdpHeightMode height_mode = vdp_get_height_mode(sms);
    const int display_height = VDP_ACTIVE_DISPLAY_HEIGHT[height_mode];

    // vblank is generated on the next line
    if (VDP.vcount == display_height + 1)
    {
        VDP.frame_interrupt_pending = true;
        z80_check_for_irq(sms);

        if (sms->vblank_callback)
        {
            sms->vblank_callback(sms->userdata, vdp_get_overscan_colour_converted(sms));
        }
    }

    // the line counter is decremented on every line within the display
    // region, including the next line.
    if (VDP.vcount <= display_height && !SMS_is_system_type_sg(sms))
    {
        if (VDP.line_counter == 0)
        {
            // reloads / fires irq (if enabled) on underflow
            VDP.line_counter = VDP.registers[0xA];
            VDP.line_interrupt_pending = true;
            z80_check_for_irq(sms);
        }
        else
        {
            VDP.line_counter--;
        }
    }

    // nmi is asserted at the start of line 261
    if (VDP.vcount == 261 && VDP.nmi_pending)
    {
        VDP.nmi_pending = false;
        z80_nmi(sms);
    }

    // end of frame.
    if (VDP.vcount == VDP_VCOUNT_MAX[sms->region])
    {
        VDP.vcount = 0;
        VDP.vertical_scroll = VDP.registers[0x9];
        VDP.line_counter = VDP.registers[0xA];
    }
}

static void vdp_tick(struct SMS_Core* sms)
{
    switch (sms->vdp.state)
    {
        case VdpState_ACTIVE:
            on_active_event(sms);
            break;

        case VdpState_BLANKING:
            on_blanking_event(sms);
            break;
    }
}

void vdp_on_event(void* user, unsigned id, unsigned late)
{
    struct SMS_Core* sms = user;
    vdp_tick(sms);
    scheduler_add(&sms->scheduler, id, NTSC_NEXT_EVENT_CYCLES[VDP.state], vdp_on_event, user);
}

void vdp_init(struct SMS_Core* sms)
{
    memset(&VDP, 0, sizeof(VDP));
    // update palette
    vdp_mark_palette_dirty(sms);

    // values on starup
    VDP.registers[0x0] = 0x04; // %00000100 (taken from VDPTEST)
    VDP.registers[0x1] = 0x20; // %00100000 (taken from VDPTEST)
    VDP.registers[0x2] = 0xF1; // %11110001 (taken from VDPTEST)
    VDP.registers[0x3] = 0xFF; // %11111111 (taken from VDPTEST)
    VDP.registers[0x4] = 0x03; // %00000011 (taken from VDPTEST)
    VDP.registers[0x5] = 0x81; // %10000001 (taken from VDPTEST)
    VDP.registers[0x6] = 0xFB; // %11111011 (taken from VDPTEST)
    VDP.registers[0x7] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0x8] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0x9] = 0x00; // %00000000 (taken from VDPTEST)
    VDP.registers[0xA] = 0xFF; // %11111111 (taken from VDPTEST)
    // vdp registers are write-only, so the the values of 0xB-0xF don't matter

    if (1) // values after bios (todo: optional bios skip)
    {
        VDP.registers[0x0] = 0x36;
        VDP.registers[0x1] = 0x80;
        VDP.registers[0x6] = 0xFB;
    }

    VDP.line_counter = 0xFF;
    VDP.state = VdpState_ACTIVE;
    VDP.nmi_pending = false;

    scheduler_add(&sms->scheduler, SchedulerID_VDP, NTSC_NEXT_EVENT_CYCLES[VDP.state], vdp_on_event, sms);
}
