#pragma once

#include <stdbool.h>
#include <stddef.h>

// these should return 0 on error.
typedef size_t (*rewind_compressor_size)(size_t src_size);
typedef size_t (*rewind_compressor)(const void* src_data, void* dst_data, size_t src_size, size_t dst_size, bool inflate_mode);

typedef struct Rewind Rewind;

// set functions to NULL to not use compression.
Rewind* rewind_init(size_t size, size_t frames_wanted, rewind_compressor compressor, rewind_compressor_size compressor_size);
void rewind_close(Rewind* rw);
void rewind_reset(Rewind* rw);

bool rewind_push(Rewind* rw, const void* data, size_t size);
bool rewind_pop(Rewind* rw, void* data, size_t size);
bool rewind_get(Rewind* rw, size_t index, void* data, size_t size);

// remove everything after index.
bool rewind_remove_after(Rewind* rw, size_t index);

// returns the number of frames.
size_t rewind_get_count(const Rewind* rw);
// returns the compressed size of a frame.
size_t rewind_get_size(const Rewind* rw, size_t index);
// returns the compressed size of the last entry, same as rewind_get_size(rw, rewind_get_count(rw) - 1).
size_t rewind_get_size_last(const Rewind* rw);
// returns the total size of all allocated memory.
size_t rewind_get_allocated_size(const Rewind* rw, bool include_internal_buffers);
