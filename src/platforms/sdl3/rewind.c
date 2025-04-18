#include "rewind.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct RewindBuffer
{
    void* data;
    size_t size;
};

struct Rewind
{
    size_t index; /* which frame we are currently in. */
    size_t count; /* how many frames we have allocated. */
    size_t max; /* max frames. */

    struct RewindBuffer* frames; /* array of compressed frames. */

    rewind_compressor compressor;
    rewind_compressor_size compressor_size;
};

// these are used if no compressor is provided.
static size_t rewind_dummy_compressor_size(size_t size)
{
    return size;
}

static size_t rewind_dummy_compressor(const void* src_data, void* dst_data, size_t src_size, size_t dst_size, bool inflate_mode)
{
    memcpy(dst_data, src_data, src_size);
    return src_size;
}

static void rewindbuffer_free(struct RewindBuffer* rwb)
{
    if (rwb->data)
    {
        free(rwb->data);
    }
    memset(rwb, 0, sizeof(*rwb));
}

// converts 0 based index to relative.
static size_t rewind_get_starting_index(const Rewind* rw, size_t index)
{
    const size_t base = (rw->index - rw->count) % rw->max;
    return (base + index) % rw->max;
}

Rewind* rewind_init(size_t size, size_t frames_wanted, rewind_compressor compressor, rewind_compressor_size compressor_size)
{
    if (!size || !frames_wanted || (compressor && !compressor_size) || (!compressor && compressor_size))
    {
        return NULL;
    }

    Rewind* rw = calloc(1, sizeof(*rw));
    if (!rw)
    {
        return NULL;
    }

    rw->max = frames_wanted;
    rw->frames = calloc(rw->max, sizeof(*rw->frames));
    rw->compressor = compressor ? compressor : rewind_dummy_compressor;
    rw->compressor_size = compressor_size ? compressor_size : rewind_dummy_compressor_size;

    if (!rw->frames)
    {
        rewind_close(rw);
        return NULL;
    }

    return rw;
}

void rewind_close(Rewind* rw)
{
    if (!rw)
    {
        return;
    }

    if (rw->frames)
    {
        size_t i;
        for (i = 0; i < rw->max; i++)
        {
            rewindbuffer_free(&rw->frames[i]);
        }

        free(rw->frames);
    }

    memset(rw, 0, sizeof(*rw));
    free(rw);
}

void rewind_reset(Rewind* rw)
{
    size_t i;
    for (i = 0; i < rw->max; i++)
    {
        rewindbuffer_free(&rw->frames[i]);
    }

    rw->count = 0;
    rw->index = 0;
}

bool rewind_push(Rewind* rw, const void* data, size_t size)
{
    if (!rw || !data)
    {
        return false;
    }

    /* allocate bound space for new compressed state. */
    struct RewindBuffer new_frame;
    new_frame.size = rw->compressor_size(size);
    new_frame.data = malloc(new_frame.size);
    if (!new_frame.data || !new_frame.size)
    {
        assert(!"failed to malloc new frame data");
        return false;
    }

    new_frame.size = rw->compressor(data, new_frame.data, size, new_frame.size, false);
    assert(new_frame.size != 0);

    /* shrink data using realloc. */
    new_frame.data = realloc(new_frame.data, new_frame.size);
    if (!new_frame.data)
    {
        assert(!"failed to realloc new frame data");
        return false;
    }

    /* remove old frame if data exists. */
    rewindbuffer_free(&rw->frames[rw->index]);

    rw->frames[rw->index] = new_frame;
    rw->index = (rw->index + 1) % rw->max;
    rw->count = rw->count < rw->max ? rw->count + 1 : rw->max;

    return true;
}

bool rewind_pop(Rewind* rw, void* data, size_t size)
{
    if (!rw || !data)
    {
        return false;
    }

    if (rw->count == 0)
    {
        assert(!"rewind pop called with no frames stored!");
        return false;
    }

    rw->index = (rw->index - 1) % rw->max;
    const size_t result = rw->compressor(rw->frames[rw->index].data, data, rw->frames[rw->index].size, size, true);
    if (!result || result != size)
    {
        assert(!"failed to uncompress");
        return false;
    }

    rewindbuffer_free(&rw->frames[rw->index]);
    rw->count--;

    return true;
}

bool rewind_get(Rewind* rw, size_t index, void* data, size_t size)
{
    if (!data)
    {
        return false;
    }

    if (index >= rw->count)
    {
        assert(!"out of bounds rewind_remove_after()");
        return false;
    }

    index = rewind_get_starting_index(rw, index);

    const size_t result = rw->compressor(rw->frames[index].data, data, rw->frames[index].size, size, true);
    if (!result || result != size)
    {
        assert(!"failed to uncompress");
        return false;
    }

    return true;
}

bool rewind_remove_after(Rewind* rw, size_t index)
{
    if (index >= rw->count)
    {
        assert(!"out of bounds rewind_remove_after()");
        return false;
    }

    index = rewind_get_starting_index(rw, index);

    for (size_t i = index; i != rw->index; i = (i + 1) % rw->max)
    {
        rewindbuffer_free(&rw->frames[i]);
        rw->count--;
    }

    rw->index = index;
    return true;
}

size_t rewind_get_count(const Rewind* rw)
{
    return rw->count;
}

size_t rewind_get_size(const Rewind* rw, size_t index)
{
    if (index >= rw->count)
    {
        assert(!"out of bounds rewind_get_size()");
        return 0;
    }

    index = rewind_get_starting_index(rw, index);
    return rw->frames[index].size;
}

size_t rewind_get_size_last(const Rewind* rw)
{
    return rewind_get_size(rw, rewind_get_count(rw) - 1);
}

size_t rewind_get_allocated_size(const Rewind* rw, bool include_internal_buffers)
{
    size_t size = 0;

    if (include_internal_buffers)
    {
        size += sizeof(*rw);
        size += rw->max * sizeof(*rw->frames);
    }

    for (size_t i = 0; i < rw->count; i++)
    {
        size += rw->frames[rewind_get_starting_index(rw, i)].size;
    }

    return size;
}
