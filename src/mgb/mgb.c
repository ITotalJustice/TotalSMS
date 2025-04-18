#include "mgb.h"
#include "ifile/ifile.h"
#include "romloader.h"
#include "directory.h"
#include "sms_types.h"
#include "util.h"
#include "ifile/cfile/cfile.h"
#include "ifile/mem/mem.h"
#include "png/png.h"

#include <string.h>
#include <stdlib.h>
#include <sms.h>
#include <assert.h>
#include <zlib.h>
#include <time.h>
#include <patch.h>

enum LoadRomType
{
    LoadRomType_FILE,
    LoadRomType_MEM,
    LoadRomType_FD,
};

struct LoadRomConfig
{
    const char* const path;
    const uint8_t* const data;
    const size_t size;
    enum LoadRomType type;
    int fd;
    bool own_fd;
};

// savestates compressed with defalte/inflate
// lz4 was considered, but it seemed silly to have 2 compression
// algorithims included in my code, although lz4 is arguably
struct StateMeta
{
    uint32_t magic;
    char platform_string[64]; // todo:
    uint32_t state_size;
    // if 0, then the state is uncompressed!
    uint32_t state_compressed_size;
    uint32_t padding;
    uint64_t timestamp;
    uint64_t playtime;
    uint8_t reserved[160];
};

enum { STATE_MAGIC = 0x536A0535 };

// struct RewindState
// {
//     struct SMS_State state;
//     uint32_t pixels[SMS_SCREEN_HEIGHT][SMS_SCREEN_WIDTH];
// };

struct mgb
{
    // set via init()
    struct SMS_Core* sms;

    void* user;
    set_on_file_callback_func on_file_cb;
    convert_pixels_to_png_format_func on_png_convert_cb;

    // [OPTIONAL]
    // folder prefixes, eg, save_folder = "/saves";
    // then the full path is /saves/rom_name.sav
    char* save_folder;
    char* rtc_folder;
    char* state_folder;

    char rom_path[0x304];
    uint8_t rom_data[SMS_ROM_SIZE_MAX];
    size_t rom_size;
    bool has_rom;

    char bios_path[0x304];
    uint8_t bios_data[SMS_ROM_SIZE_MAX];
    size_t bios_size;
    bool has_bios;

    // uint8_t sram_data[SMS_SAVE_SIZE_MAX];
    // size_t sram_size;
    // bool has_sram;
};


static void free_game(void);
static void loadsave(void);
static bool loadrom(const struct LoadRomConfig* config);

// globals
static struct mgb mgb = {0};
// globals end

static uint8_t* apply_patch(
    const uint8_t* src_data, size_t src_size,
    const uint8_t* patch_data, size_t patch_size,
    size_t* out_size
) {
    enum PatchType type;
    if (PatchError_OK != patch_get_type(&type, patch_data, patch_size))
    {
        return NULL;
    }

    if (PatchError_OK != patch_get_size(type, out_size, src_size, patch_data, patch_size))
    {
        return NULL;
    }

    uint8_t* out = malloc(*out_size);
    if (!out)
    {
        return NULL;
    }

    if (PatchError_OK != patch_apply(type, out, *out_size, src_data, src_size, patch_data, patch_size))
    {
        free(out);
        return NULL;
    }

    // patch worked, out is now pointing to allocated data.
    // remember to call free when done!
    return out;
}

static void free_game(void)
{
    mgb_save_save_file(NULL);
    // mgb_save_state_file(NULL);
    mgb.has_rom = false;
}

static void loadsave(void)
{
    const struct SafeString ss = util_create_save_path(mgb.save_folder, mgb.rom_path);

    if (ss_valid(&ss))
    {
        IFile_t* file = icfile_open(ss.str, IFileMode_READ, 0);

        if (file)
        {
            const size_t save_size = SMS_SRAM_SIZE_MAX;

            // we allow for bigger sizes because it might be an vba save
            if (ifile_size(file) == save_size)
            {
                if (ifile_read(file, mgb.sms->cart.ram, save_size))
                {
                    if (mgb.on_file_cb)
                    {
                        mgb.on_file_cb(mgb.user, ss.str, CallbackType_LOAD_SAVE, true);
                    }
                    mgb_log("[MGB] loaded save: %s\n", ss.str);
                }
                else
                {
                    mgb_log_err("[MGB] failed to load save: %s\n", ss.str);
                }
            }
            else
            {
                mgb_log_err("[MGB] failed to load save: %s invalid size\n", ss.str);
            }

            ifile_close(file);
        }
    }
}

static bool loadbios(const struct LoadRomConfig* config)
{
    IFile_t* romloader = NULL;

    switch (config->type)
    {
        case LoadRomType_FILE:
            romloader = romloader_open(config->path);
            break;

        case LoadRomType_MEM:
            romloader = romloader_open_mem(config->path, config->data, config->size);
            break;

        case LoadRomType_FD:
            romloader = romloader_open_fd(config->fd, config->own_fd, config->path);
            break;
    }

    if (!romloader)
    {
        goto fail;
    }

    mgb.bios_size = ifile_size(romloader);

    if (!mgb.bios_size || mgb.bios_size > sizeof(mgb.bios_data))
    {
        mgb_log_err("[MGB] bios size is bad %zu\n", mgb.bios_size);
        goto fail;
    }

    if (!ifile_read(romloader, mgb.bios_data, mgb.bios_size))
    {
        mgb_log_err("[MGB] fail to read size: %zu\n", mgb.bios_size);
        goto fail;
    }

    if (!SMS_loadbios(mgb.sms, mgb.bios_data, mgb.bios_size))
    {
        mgb_log_err("[MGB] fail to gb load rom\n");
        goto fail;
    }

    // free everything
    ifile_close(romloader);
    romloader = NULL;

    // save the path
    strncpy(mgb.bios_path, config->path, sizeof(mgb.bios_path) - 1);

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_LOAD_BIOS, true);
    }

    mgb.has_bios = true;

    // todo: finish support of this in sms.c
#if 0
    // the bios may have a rom embeded.
    if (SMS_has_rom(mgb.sms))
    {
        // save the path
        strncpy(mgb.rom_path, config->path, sizeof(mgb.rom_path) - 1);

        // report that the rom has loade before calling the callback.
        mgb.has_rom = true;

        if (mgb.on_file_cb)
        {
            mgb.on_file_cb(mgb.user, config->path, CallbackType_LOAD_ROM, true);
        }

        // try loading any saves if possible
        loadsave();
    }
#endif

    return true;

fail:
    if (romloader)
    {
        ifile_close(romloader);
        romloader = NULL;
    }

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_LOAD_BIOS, false);
    }

    mgb.has_bios = false;

    return false;
}

static bool loadrom(const struct LoadRomConfig* config)
{
    IFile_t* romloader = NULL;

    if (mgb_has_rom())
    {
        free_game();
    }

    switch (config->type)
    {
        case LoadRomType_FILE:
            romloader = romloader_open(config->path);
            break;

        case LoadRomType_MEM:
            romloader = romloader_open_mem(config->path, config->data, config->size);
            break;

        case LoadRomType_FD:
            romloader = romloader_open_fd(config->fd, config->own_fd, config->path);
            break;
    }

    if (!romloader)
    {
        goto fail;
    }

    mgb.rom_size = ifile_size(romloader);

    if (!mgb.rom_size || mgb.rom_size > sizeof(mgb.rom_data))
    {
        mgb_log_err("[MGB] size is bad %zu\n", mgb.rom_size);
        goto fail;
    }

    if (!ifile_read(romloader, mgb.rom_data, mgb.rom_size))
    {
        mgb_log_err("[MGB] fail to read size: %zu\n", mgb.rom_size);
        goto fail;
    }


    const enum ExtensionType type = util_get_extension_type(config->path, ExtensionOffsetType_LAST);
    int system_hint = -1;

    if (type == ExtensionType_SMS)
    {
        system_hint = SMS_System_SMS;
    }
    else if (type == ExtensionType_GG)
    {
        system_hint = SMS_System_GG;
    }
    else if (type == ExtensionType_SG)
    {
        system_hint = SMS_System_SG1000;
    }

    if (!SMS_loadromEx(mgb.sms, mgb.rom_data, mgb.rom_size, system_hint, -1, -1))
    {
        mgb_log_err("[MGB] fail to gb load rom\n");
        goto fail;
    }

    // free everything
    ifile_close(romloader);
    romloader = NULL;

    // save the path
    strncpy(mgb.rom_path, config->path, sizeof(mgb.rom_path) - 1);

    // report that the rom has loade before calling the callback.
    mgb.has_rom = true;

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_LOAD_ROM, true);
    }

    // try loading any saves if possible
    loadsave();
    // mgb_load_state_file(NULL);

    return true;

fail:
    if (romloader)
    {
        ifile_close(romloader);
        romloader = NULL;
    }

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_LOAD_ROM, false);
    }

    mgb.has_rom = false;

    return false;
}

bool mgb_load_bios_file(const char* path)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .type = LoadRomType_FILE
    };

    return loadbios(&config);
}

bool mgb_load_rom_file(const char* path)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .type = LoadRomType_FILE
    };

    return loadrom(&config);
}

bool mgb_load_rom_fd(int fd, bool own, const char* path)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .type = LoadRomType_FD,
        .fd = fd,
        .own_fd = own,
    };

    return loadrom(&config);
}

bool mgb_load_rom_data(const char* path, const uint8_t* data, size_t size)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .data = data,
        .size = size,
        .type = LoadRomType_MEM
    };

    return loadrom(&config);
}

bool mgb_save_save_file(const char* path)
{
    if (!mgb_has_rom())
    {
        return false;
    }

    if (SMS_used_sram(mgb.sms) && SMS_is_sram_dirty(mgb.sms, true))
    {
        struct SafeString ss = {0};

        if (path)
        {
            strncpy(ss.str, path, sizeof(ss.str) - 1);
        }
        else
        {
            ss = util_create_save_path(mgb.save_folder, mgb.rom_path);
        }

        if (ss_valid(&ss))
        {
            IFile_t* file = icfile_open(ss.str, IFileMode_WRITE, 0);

            if (file)
            {
                const size_t save_size = SMS_SRAM_SIZE_MAX;
                const bool result = ifile_write(file, mgb.sms->cart.ram, save_size);
                ifile_close(file);

                if (result)
                {
                    mgb_log("[MGB] saved game: %s size: %zu\n", ss.str, save_size);
                    if (mgb.on_file_cb)
                    {
                        mgb.on_file_cb(mgb.user, ss.str, CallbackType_SAVE_SAVE, true);
                    }
                    return true;
                }
                else
                {
                    mgb_log_err("[MGB] failed to write save: %s\n", ss.str);
                    goto fail;
                }
            }
            else
            {
                mgb_log_err("[MGB] failed to open savefile for writing: %s\n", ss.str);
                goto fail;
            }
        }
        else
        {
            mgb_log_err("[MGB] invalid save!\n");
        }

    fail:
        if (mgb.on_file_cb)
        {
            mgb.on_file_cb(mgb.user, ss.str, CallbackType_SAVE_SAVE, false);
        }
        return false;
    }

    return true;
}

bool mgb_save_state_file(const char* path)
{
    IFile_t* file = NULL;
    void* state = NULL;
    struct StateMeta meta = {0};
    struct SafeString ss = {0};

    if (!mgb_has_rom())
    {
        mgb_log_err("[MGB] tried to save state without rom\n");
        goto fail;
    }

    if (path)
    {
        strncpy(ss.str, path, sizeof(ss.str) - 1);
    }
    else
    {
        ss = util_create_state_path(mgb_get_state_folder(), mgb_rom_path());
    }

    if (!ss_valid(&ss))
    {
        mgb_log_err("ss invalid\n");
        goto fail;
    }

    file = icfile_open(ss.str, IFileMode_WRITE, 0);
    if (!file)
    {
        mgb_log_err("[MGB] failed to open\n");
        goto fail;
    }

    meta.magic = STATE_MAGIC;
    strcpy(meta.platform_string, "linux dev");
    meta.timestamp = time(NULL);
    meta.state_size = SMS_get_state_size(mgb.sms, NULL);
    state = malloc(meta.state_size);

    if (!SMS_savestate(mgb.sms, state, meta.state_size, NULL))
    {
        mgb_log_err("[MGB] failed to state\n");
        goto fail;
    }

    // compress state file
    {
        const uLong bound = compressBound(meta.state_size);
        uLongf dst_len = bound;
        void* compressed_state = malloc(dst_len);
        if (Z_OK != compress(compressed_state, &dst_len, state, meta.state_size))
        {
            free(compressed_state);
            goto fail;
        }
        meta.state_compressed_size = dst_len;

        free(state);
        state = realloc(compressed_state, meta.state_compressed_size);
    }

    if (mgb.on_png_convert_cb)
    {
        int pixels_w;
        int pixels_h;
        int pixels_channels;
        void* pixels = mgb.on_png_convert_cb(mgb.user, &pixels_w, &pixels_h, &pixels_channels);
        if (pixels)
        {
            unsigned png_size;
            void* png = png_compress(pixels, pixels_w, pixels_h, pixels_channels, &png_size);
            if (png)
            {
                ifile_write(file, png, png_size);
                free(png);
            }
            free(pixels);
        }
    }

    // const uint32_t meta_offset = ifile_tell(file);
    if (!ifile_write(file, &meta, sizeof(meta)))
    {
        mgb_log_err("[MGB] failed to write\n");
        goto fail;
    }

    if (!ifile_write(file, state, meta.state_compressed_size))
    {
        mgb_log_err("[MGB] failed to write\n");
        goto fail;
    }

    ifile_close(file);
    free(state);
    mgb_log("[MGB] saved to save state file: %s\n", ss.str);

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, ss.str, CallbackType_SAVE_STATE, true);
    }

    return true;

fail:
    if (file)
    {
        ifile_close(file);
    }

    if (state)
    {
        free(state);
    }

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, ss.str, CallbackType_SAVE_STATE, false);
    }

    mgb_log_err("[MGB] failed to save state file: %s\n", ss.str);

    return false;
}

bool mgb_load_state_file(const char* path)
{
    IFile_t* file = NULL;
    void* state = NULL;
    void* state_compressed = NULL;
    struct StateMeta meta = {0};
    struct SafeString ss = {0};

    if (!mgb_has_rom())
    {
        mgb_log_err("[MGB] tried to load state without rom\n");
        goto fail;
    }

    if (path)
    {
        strncpy(ss.str, path, sizeof(ss.str) - 1);
    }
    else
    {
        ss = util_create_state_path(mgb_get_state_folder(), mgb_rom_path());
    }

    if (!ss_valid(&ss))
    {
        goto fail;
    }

    mgb_log("[MGB] trying to load state from: %s\n", ss.str);

    file = icfile_open(ss.str, IFileMode_READ, 0);
    if (!file)
    {
        mgb_log_err("[MGB] failed to open file: %s\n", ss.str);
        goto fail;
    }

    // check for png data by reading header
    uint8_t png_header[PNG_HEADER_SIZE];
    if (!ifile_read(file, png_header, sizeof(png_header)))
    {
        mgb_log_err("[MGB] failed to read file: %s\n", ss.str);
        goto fail;
    }

    const uint32_t png_size = png_get_absolute_size(png_header, sizeof(png_header));

    // skip over png data
    if (!ifile_seek(file, png_size, SEEK_SET))
    {
        mgb_log_err("[MGB] failed to read file: %s\n", ss.str);
        goto fail;
    }

    // read meta data
    if (!ifile_read(file, &meta, sizeof(meta)))
    {
        mgb_log_err("[MGB] failed to read file: %s\n", ss.str);
        goto fail;
    }

    // todo: check values here...
    // todo: compare state size with sms
    if (meta.magic != STATE_MAGIC)
    {
        mgb_log("bad state magic...\n");
        goto fail;
    }

    const time_t timestamp = meta.timestamp;
    const struct tm* tm = localtime(&timestamp);
    if (tm) {
        mgb_log("savestate date: %02u/%02u/%04u %02u:%02u\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min);
    } else {
        mgb_log_err("bad time... %zu\n", meta.timestamp);
    }

    uLongf dst_len = meta.state_size;
    state_compressed = malloc(meta.state_compressed_size);

    if (!ifile_read(file, state_compressed, meta.state_compressed_size))
    {
        mgb_log_err("[MGB] failed to read file: %s\n", ss.str);
        goto fail;
    }

    state = malloc(dst_len);

    if (Z_OK != uncompress(state, &dst_len, state_compressed, meta.state_compressed_size))
    {
        mgb_log_err("failed to decompress state\n");
        goto fail;
    }

    // if (!SMS_loadstate(sms, state, dst_len))
    if (!SMS_loadstate(mgb.sms, state, SMS_get_state_size(mgb.sms, NULL), NULL))
    {
        mgb_log_err("[MGB] GB failed to loadstate: %s\n", ss.str);
        goto fail;
    }

    ifile_close(file);
    free(state);
    free(state_compressed);

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, ss.str, CallbackType_LOAD_STATE, true);
    }

    return true;

fail:
    if (file)
    {
        ifile_close(file);
    }

    if (state)
    {
        free(state);
    }

    if (state_compressed)
    {
        free(state_compressed);
    }

    mgb_log_err("[MGB] failed to load state from: %s\n", ss.str);

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, ss.str, CallbackType_LOAD_STATE, false);
    }

    return false;
}

static bool patch_rom(const struct LoadRomConfig* config)
{
    IFile_t* file = NULL;
    uint8_t* patch_data = NULL;

    if (!mgb_has_rom())
    {
        mgb_log_err("[mgb] no rom\n");
        goto fail;
    }

    switch (config->type)
    {
        case LoadRomType_FILE:
            file = icfile_open(config->path, IFileMode_READ, 0);
            break;

        case LoadRomType_MEM:
            file = imem_open_const(config->data, config->size, IFileMode_READ, 0);
            break;

        case LoadRomType_FD:
            file = icfile_open_fd(config->fd, config->own_fd, IFileMode_READ, 0);
            break;
    }

    if (!file)
    {
        mgb_log_err("[mgb] no file\n");
        goto fail;
    }

    const size_t patch_size = ifile_size(file);
    patch_data = malloc(patch_size);

    if (!patch_size || !patch_data)
    {
        mgb_log_err("[MGB] patch size is bad %zu\n", patch_size);
        goto fail;
    }

    if (!ifile_read(file, patch_data, patch_size))
    {
        mgb_log_err("[MGB] fail to read size: %zu\n", patch_size);
        goto fail;
    }

    size_t new_size;
    uint8_t* data = apply_patch(mgb.rom_data, mgb.rom_size, patch_data, patch_size, &new_size);

    if (!data || !new_size || new_size > SMS_ROM_SIZE_MAX)
    {
        goto fail;
    }

    // the below is a bit of a hack to work around sms mapper_init memset
    // cart ram. so we dump the ram and then re-load it.
    mgb_save_save_file(NULL);

    if (!SMS_loadromEx(mgb.sms, data, new_size, -1, -1, -1))
    {
        goto fail;
    }

    memcpy(mgb.rom_data, data, new_size);
    mgb.rom_size = new_size;

    loadsave();

    // free everything
    ifile_close(file);
    free(patch_data);

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_PATCH_ROM, true);
    }

    return true;

fail:
    if (file)
    {
        ifile_close(file);
    }

    if (patch_data)
    {
        free(patch_data);
    }

    if (mgb.on_file_cb)
    {
        mgb.on_file_cb(mgb.user, config->path, CallbackType_PATCH_ROM, false);
    }

    return false;
}

bool mgb_patch_rom_file(const char* path)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .type = LoadRomType_FILE
    };

    return patch_rom(&config);
}

bool mgb_patch_rom_fd(int fd, bool own, const char* path)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .type = LoadRomType_FD,
        .fd = fd,
        .own_fd = own,
    };

    return patch_rom(&config);
}

bool mgb_patch_rom_data(const char* path, const uint8_t* data, size_t size)
{
    const struct LoadRomConfig config =
    {
        .path = path,
        .data = data,
        .size = size,
        .type = LoadRomType_MEM
    };

    return patch_rom(&config);
}

bool mgb_init(struct SMS_Core* sms)
{
    memset(&mgb, 0, sizeof(struct mgb));
    mgb.sms = sms;
    return true;
}

void mgb_exit(void)
{
    #if !defined(ANDROID) && !defined(EMSCRIPTEN)
    if (mgb.sms)
    {
        free_game();
    }
    #endif

    if (mgb.save_folder)
    {
        free(mgb.save_folder);
        mgb.save_folder = NULL;
    }
    if (mgb.rtc_folder)
    {
        free(mgb.rtc_folder);
        mgb.rtc_folder = NULL;
    }
    if (mgb.state_folder)
    {
        free(mgb.state_folder);
        mgb.state_folder = NULL;
    }
}

bool mgb_has_rom(void)
{
    return mgb.has_rom;
}

const char* mgb_rom_path(void)
{
    return mgb.rom_path;
}

void mgb_set_save_folder(const char* path)
{
    mgb.save_folder = strdup(path);
    directory_create(mgb.save_folder, DIRECTORY_RWX);
}

void mgb_set_rtc_folder(const char* path)
{
    mgb.rtc_folder = strdup(path);
    directory_create(mgb.rtc_folder, DIRECTORY_RWX);
}

void mgb_set_state_folder(const char* path)
{
    mgb.state_folder = strdup(path);
    directory_create(mgb.state_folder, DIRECTORY_RWX);
}

const char* mgb_get_save_folder(void)
{
    return mgb.save_folder;
}

const char* mgb_get_rtc_folder(void)
{
    return mgb.rtc_folder;
}

const char* mgb_get_state_folder(void)
{
    return mgb.state_folder;
}

void mgb_set_userdata(void* user)
{
    mgb.user = user;
}

void mgb_set_on_file_callback(set_on_file_callback_func cb)
{
    mgb.on_file_cb = cb;
}

void mgb_set_on_convert_pixels_to_png_format(convert_pixels_to_png_format_func cb)
{
    mgb.on_png_convert_cb = cb;
}
