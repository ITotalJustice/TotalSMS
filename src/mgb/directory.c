#include "directory.h"

// todo: windows version!
#ifdef WIN32
#include <Windows.h>
#error "directory_scan() not yet implemented on windows!"

// this isn't perfect yet as it needs to check for '\' and '/'

int directory_create(const char* path, uint32_t flags) {
    char* str = NULL;
    char* ptr = NULL;
    size_t len = 0;
    // int mode = 0; // unused currently...

    if (!path)
        return -1;

    //if (!mode)
    //    return -1;

    len = strlen(path);
    if (!len) // sanity check
        return -1;

    str = strdup(path);
    if (!str)
        return -1;

    if (str[len - 1] == '/')
        str[len - 1] = '\0';

    for (ptr = str + 1; *ptr; ptr++) {
        if (*ptr == '/') {
            *ptr = '\0';
            if (!CreateDirectory(str, NULL)) {
                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    goto error;
                }
            }
            *ptr = '/';
        }
    }

    if (!CreateDirectory(str, NULL)) {
    error:
        free(str);
        return -1;
    }

    free(str);
    return 0;
}
#else

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

size_t directory_scan(const char* path, struct DirEntry* entries, size_t count)
{
    if (!path || !entries || !count)
    {
        return 0;
    }

    DIR* dir = NULL;
    struct dirent* d = NULL;
    size_t entries_found = 0;

    dir = opendir(path);
    if (!dir)
    {
        return 0;
    }

    while ((d = readdir(dir)) && entries_found < count)
    {
        // skip hiden files and folders as well as "." and ".."
        if (d->d_name[0] == '.')
        {
            continue;
        }

        strcpy(entries[entries_found].name, d->d_name);
        #if defined(_DIRENT_HAVE_D_TYPE)
        if (d->d_type == DT_DIR)
        {
            entries[entries_found].type = DirEntryType_FOLDER;
        }
        else
        {
            entries[entries_found].type = DirEntryType_FILE;
        }
        #else
        struct stat st;
        stat(ss_build("%s/%s", path, d->d_name).str, &st);
		if (S_ISDIR(st.st_mode))
        {
            entries[entries_found].type = DirEntryType_FOLDER;
		}
        else
        {
            entries[entries_found].type = DirEntryType_FILE;
        }
        // todo: stat each file (although it's super slow)
        #endif
        entries_found++;
    }

    closedir(dir);
    return entries_found;
}

int directory_create(const char* path, uint32_t flags) {
    char* str = NULL;
    char *ptr = NULL;
    size_t len = 0;
    int mode = 0;

    if (!path)
        return -1;

    if (flags & DIRECTORY_READ)
        mode |= S_IRUSR | S_IRGRP | S_IROTH;

    if (flags & DIRECTORY_WRITE)
        mode |= S_IWUSR | S_IWGRP | S_IWOTH;

    if (flags & DIRECTORY_EXECUTE)
        mode |= S_IXUSR | S_IXGRP | S_IXOTH;

    if (!mode)
        return -1;

    len = strlen(path);
    if (!len) // sanity check
        return -1;

    str = strdup(path);
    if (!str)
        return -1;

    if (str[len - 1] == '/')
        str[len - 1] = '\0';

    for (ptr = str+1; *ptr; ptr++) {
        if (*ptr == '/') {
            *ptr = '\0';
            if (mkdir(str, mode) != 0 && errno != EEXIST)
                goto error;
            *ptr = '/';
        }
    }

    if (mkdir(str, mode) != 0 && errno != EEXIST) {
error:
        free(str);
        return -1;
    }

    free(str);
    return 0;
}
#endif



// sdl2 version, to be upstreamed soon
#if 0
int SDL_CreateDirectory(const char* path, Uint32 flags) {
    char* str = NULL;
    char *ptr = NULL;
    size_t len = 0;
    int mode = 0;

    if (!path)
        return SDL_InvalidParamError("path");

    if (flags & SDL_DIRECTORY_READ)
        mode |= S_IRUSR | S_IRGRP | S_IROTH;

    if (flags & SDL_DIRECTORY_WRITE)
        mode |= S_IWUSR | S_IWGRP | S_IWOTH;

    if (flags & SDL_DIRECTORY_EXECUTE)
        mode |= S_IXUSR | S_IXGRP | S_IXOTH;

    if (!mode)
        return SDL_InvalidParamError("flags");

    len = SDL_strlen(path);
    if (!len) // sanity check
        return -1;

    str = SDL_strdup(path);
    if (!str)
        return SDL_OutOfMemory();

    if (str[len - 1] == '/')
        str[len - 1] = '\0';

    for (ptr = str+1; *ptr; ptr++) {
        if (*ptr == '/') {
            *ptr = '\0';
            if (mkdir(str, mode) != 0 && errno != EEXIST)
                goto error;
            *ptr = '/';
        }
    }

    if (mkdir(str, mode) != 0 && errno != EEXIST) {
error:
        SDL_SetError("Couldn't create directory '%s': '%s'", str, strerror(errno));
        SDL_free(str);
        return -1;
    }

    SDL_free(str);
    return 0;
}
#endif
