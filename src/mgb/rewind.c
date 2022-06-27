#include "rewind.h"
#include "mgb.h"
#include "util.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// matching data gets xored to 0, which compresses really well
static void xor(const uint8_t* keydata, size_t keysize, uint8_t* frame, size_t framesize)
{
    UNUSED(framesize);
    assert(keysize == framesize && "[XOR] key and frame size doe not match, this is unsupported");

    for (size_t i = 0; i < keysize; i++)
    {
        frame[i] ^= keydata[i];
    }
}

// allocates new data for keyframe
static void rewindframe_init(struct RewindFrame* rwf, const uint8_t* keydata, size_t keysize)
{
    memset(rwf, 0, sizeof(struct RewindFrame));

    rwf->keyframe.data = malloc(keysize);
    rwf->keyframe.size = keysize;
    memcpy(rwf->keyframe.data, keydata, keysize);
}

static void rewindframe_close(struct RewindFrame* rwf)
{
    if (rwf->keyframe.data)
    {
        free(rwf->keyframe.data);
    }

    for (size_t i = 0; i < rwf->count; i++)
    {
        assert(rwf->data[i].data);
        free(rwf->data[i].data);
    }

    memset(rwf, 0, sizeof(struct RewindFrame));
}

static bool rewindframe_is_empty(const struct RewindFrame* rwf)
{
    return rwf->count == 0;
}

static bool rewindframe_is_full(const struct RewindFrame* rwf)
{
    return rwf->count == REWIND_FRAME_ENTRY_COUNT;
}

static bool rewindframe_push(struct RewindFrame* rwf, const uint8_t* framedata, size_t framesize, rewind_compressor_func_t compressor, rewind_compressor_size_func_t compressor_size)
{
    // we can't push any more frames!
    assert(rewindframe_is_full(rwf) == false && "[RWF] tried to push frame when full");

    uint8_t* compressed_data = (uint8_t*)malloc(framesize);
    size_t compressed_size = framesize;
    memcpy(compressed_data, framedata, framesize);

    xor(rwf->keyframe.data, rwf->keyframe.size, compressed_data, framesize);

    if (compressor && compressor_size)
    {
        compressed_size = compressor_size(framesize);
        void* buf = malloc(compressed_size);

        memcpy(buf, compressed_data, framesize);
        compressed_size = compressor(buf, compressed_size, compressed_data, framesize, 0);

        if (compressed_size == (size_t)-1)
        {
            assert(compressed_size && "failed to compress");
            free(compressed_data);
            free(buf);
            return false;
        }

        free(compressed_data);
        compressed_data = realloc(buf, compressed_size);
        mgb_log("compress result: %zu\n", compressed_size);
    }

    rwf->data[rwf->count].data = compressed_data;
    rwf->data[rwf->count].size = framesize;
    rwf->data[rwf->count].compressed_size = compressed_size;
    assert(rwf->data[rwf->count].size && rwf->data[rwf->count].compressed_size);
    rwf->count++;

    return true;
}

static bool rewindframe_pop(struct RewindFrame* rwf, uint8_t* outdata, size_t outsize, rewind_compressor_func_t compressor)
{
    // can't pop any more frames!
    assert(rewindframe_is_empty(rwf) == false && "[RWF] tried to pop frame when empty");

    struct RewindFrameEntry* entry = &rwf->data[rwf->count - 1];

    if (entry->size != outsize)
    {
        assert(entry->size == outsize);
        return false;
    }

    if (compressor)
    {
        const size_t result = compressor(outdata, outsize, entry->data, entry->compressed_size, 1);
        mgb_log("decompress result: %zu size: %zu\n", result, outsize);
        assert(result && "failed to decompress");
        if (!result)
        {
            return false;
        }
    }
    else
    {
        memcpy(outdata, entry->data, outsize);
    }

    xor(rwf->keyframe.data, rwf->keyframe.size, outdata, outsize);

    free(entry->data);
    memset(entry, 0, sizeof(struct RewindFrameEntry));

    rwf->count--;
    return true;
}

static size_t rewind_calculate_seconds(size_t seconds_wanted)
{
    size_t a = REWIND_FRAME_ENTRY_COUNT / 60;
    return seconds_wanted < a ? a : seconds_wanted / a;
}

bool rewind_init(struct Rewind* rw, size_t seconds)
{
    if (!rw || !seconds)
    {
        return false;
    }

    memset(rw, 0, sizeof(struct Rewind));

    rw->max = rewind_calculate_seconds(seconds);
    rw->frames = calloc(rw->max, sizeof(struct RewindFrame));

    return true;
}

void rewind_close(struct Rewind* rw)
{
    if (rw->frames)
    {
        for (size_t i = 0; i < rw->count; i++)
        {
            rewindframe_close(&rw->frames[i]);
        }

        free(rw->frames);
    }

    memset(rw, 0, sizeof(*rw));
}

void rewind_add_compressor(struct Rewind* rw, rewind_compressor_func_t compressor, rewind_compressor_size_func_t size)
{
    rw->compressor = compressor;
    rw->compressor_size = size;
}

bool rewind_pop(struct Rewind* rw, uint8_t* data, size_t size)
{
check_again:
    if (rw->count == 0)
    {
        return false;
    }

    if (rw->frames[rw->index].count == 0)
    {
        rewindframe_close(&rw->frames[rw->index]);
        rw->index = rw->index ? rw->index - 1 : rw->max - 1;
        rw->count--;
        goto check_again;
    }

    return rewindframe_pop(&rw->frames[rw->index], data, size, rw->compressor);
}

bool rewind_push(struct Rewind* rw, const uint8_t* data, size_t size)
{
    bool new_keyframe = false;

    // we are at the start
    if (rewindframe_is_empty(&rw->frames[rw->index]))
    {
        new_keyframe = true;
    }
    // we are at the end
    else if (rewindframe_is_full(&rw->frames[rw->index]))
    {
        rw->index = (rw->index + 1) % (rw->max);
        new_keyframe = true;
    }

    // push new keyframe
    if (new_keyframe)
    {
        rewindframe_close(&rw->frames[rw->index]);
        rewindframe_init(&rw->frames[rw->index], data, size);
        rw->count = rw->count < rw->max ? rw->count + 1 : rw->max;
    }

    // push new entry
    return rewindframe_push(&rw->frames[rw->index], data, size, rw->compressor, rw->compressor_size);
}
