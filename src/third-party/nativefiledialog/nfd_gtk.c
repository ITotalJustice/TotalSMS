/*
  Native File Dialog

  http://www.frogtoss.com/labs
*/

#define NFD_MAX_STRLEN 256
#define _NFD_UNUSED(x) ((void)x)
#define NFD_UTF8_BOM "\xEF\xBB\xBF"

// NFD_COMMON
// Disable warning using strncat()
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "nfd.h"

#define FTG_IMPLEMENT_CORE
#include "ftg_core.h"


static char g_errorstr[NFD_MAX_STRLEN] = {0};

/* internal routines */
void* NFDi_Malloc(size_t bytes);
void NFDi_SetError(const char* msg);
int NFDi_SafeStrncpy(char* dst, const char* src, size_t maxCopy);
int32_t NFDi_UTF8_Strlen(const nfdchar_t* str);
int NFDi_IsFilterSegmentChar(char ch);
void NFDi_SplitPath(const char* path, const char** out_dir, const char** out_filename);

/* public routines */

const char*
NFD_GetError(void)
{
    return g_errorstr;
}

size_t
NFD_PathSet_GetCount(const nfdpathset_t* pathset)
{
    assert(pathset);
    return pathset->count;
}

nfdchar_t*
NFD_PathSet_GetPath(const nfdpathset_t* pathset, size_t num)
{
    assert(pathset);
    assert(num < pathset->count);

    return pathset->buf + pathset->indices[num];
}

void
NFD_PathSet_Free(nfdpathset_t* pathset)
{
    assert(pathset);
    free(pathset->indices);
    free(pathset->buf);
}

void
*NFDi_Calloc( size_t num, size_t size )
{
    void *ptr = calloc(num, size);
    if ( !ptr )
        NFDi_SetError("NFDi_Calloc failed.");

    return ptr;
}

/* internal routines */

void*
NFDi_Malloc(size_t bytes)
{
    void* ptr = malloc(bytes);
    if (!ptr)
        NFDi_SetError("NFDi_Malloc failed.");

    return ptr;
}

void
NFDi_SetError(const char* msg)
{
    int bTruncate = NFDi_SafeStrncpy(g_errorstr, msg, NFD_MAX_STRLEN);
    assert(!bTruncate);
    _NFD_UNUSED(bTruncate);
}


int
NFDi_SafeStrncpy(char* dst, const char* src, size_t maxCopy)
{
    size_t n = maxCopy;
    char*  d = dst;

    assert(src);
    assert(dst);

    while (n > 0 && *src != '\0') {
        *d++ = *src++;
        --n;
    }

    /* Truncation case -
       terminate string and return true */
    if (n == 0) {
        dst[maxCopy - 1] = '\0';
        return 1;
    }

    /* No truncation.  Append a single NULL and return. */
    *d = '\0';
    return 0;
}


/* adapted from microutf8 */
int32_t
NFDi_UTF8_Strlen(const nfdchar_t* str)
{
    /* This function doesn't properly check validity of UTF-8 character
    sequence, it is supposed to use only with valid UTF-8 strings. */

    int32_t       character_count = 0;
    int32_t       i = 0; /* Counter used to iterate over string. */
    nfdchar_t     maybe_bom[4];
    unsigned char c;

    /* If there is UTF-8 BOM ignore it. */
    if (strlen(str) > 2) {
        strncpy(maybe_bom, str, 3);
        maybe_bom[3] = 0;
        if (strcmp(maybe_bom, (nfdchar_t*)NFD_UTF8_BOM) == 0)
            i += 3;
    }

    while (str[i]) {
        c = (unsigned char)str[i];
        if (c >> 7 == 0) {
            /* If bit pattern begins with 0 we have ascii character. */
            ++character_count;
        } else if (c >> 6 == 3) {
            /* If bit pattern begins with 11 it is beginning of UTF-8 byte sequence. */
            ++character_count;
        } else if (c >> 6 == 2)
            ; /* If bit pattern begins with 10 it is middle of utf-8 byte sequence. */
        else {
            /* In any other case this is not valid UTF-8. */
            return -1;
        }
        ++i;
    }

    return character_count;
}

int
NFDi_IsFilterSegmentChar(char ch)
{
    return (ch == ',' || ch == ';' || ch == '\0');
}

void
NFDi_SplitPath(const char* path, const char** out_dir, const char** out_filename)
{
    // test filesystem to early out test on 'c:\path', which can't be
    // determined to not be a filename called `path` at `c:\`
    // otherwise.
    if (ftg_is_dir(path)) {
        *out_dir = path;
        *out_filename = NULL;
        return;
    }

    const char* filename = ftg_get_filename_from_path(path);
    if (filename[0]) {
        *out_filename = filename;
    }
    *out_dir = path;
}
// NFD_COMMON_END

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include <ctype.h>
#if defined(GDK_WINDOWING_X11)
#    include <gdk/gdkx.h>
#endif /* defined(GDK_WINDOWING_X11) */

#if defined(GDK_WINDOWING_X11) && !defined(GDK_IS_X11_DISPLAY)
#define GDK_IS_X11_DISPLAY(display) 1
#endif

const char INIT_FAIL_MSG[] = "gtk_init_check failed to initilaize GTK+";

#ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

static void
AddTypeToFilterName(const char* typebuf, char* filterName, size_t bufsize)
{
    const char SEP[] = ", ";

    size_t len = strlen(filterName);
    if (len != 0) {
        strncat(filterName, SEP, bufsize - len - 1);
        len += strlen(SEP);
    }

    strncat(filterName, typebuf, bufsize - len - 1);
}

static void
AddFiltersToDialog(GtkWidget* dialog, const char* filterList)
{
    GtkFileFilter* filter;
    char           typebuf[NFD_MAX_STRLEN] = {0};
    const char*    p_filterList = filterList;
    char*          p_typebuf = typebuf;
    char           filterName[NFD_MAX_STRLEN] = {0};

    if (!filterList || strlen(filterList) == 0)
        return;

    filter = gtk_file_filter_new();
    while (1) {
        if (NFDi_IsFilterSegmentChar(*p_filterList)) {
            char typebufWildcard[NFD_MAX_STRLEN + 3];
            /* add another type to the filter */
            assert(strlen(typebuf) > 0);
            assert(strlen(typebuf) < NFD_MAX_STRLEN - 1);

            snprintf(typebufWildcard, NFD_MAX_STRLEN + 3, "*.%s", typebuf);
            AddTypeToFilterName(typebuf, filterName, NFD_MAX_STRLEN);

            gtk_file_filter_add_pattern(filter, typebufWildcard);

            p_typebuf = typebuf;
            memset(typebuf, 0, sizeof(char) * NFD_MAX_STRLEN);
        }

        if (*p_filterList == ';' || *p_filterList == '\0') {
            /* end of filter -- add it to the dialog */

            gtk_file_filter_set_name(filter, filterName);
            gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

            filterName[0] = '\0';

            if (*p_filterList == '\0')
                break;

            filter = gtk_file_filter_new();
        }

        if (!NFDi_IsFilterSegmentChar(*p_filterList)) {
            *p_typebuf = *p_filterList;
            p_typebuf++;
        }

        p_filterList++;
    }

    /* always append a wildcard option to the end*/

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "*.*");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}

static void
GetFirstFilterExtension(const char* filterList, char outExt[NFD_MAX_STRLEN])
{
    const char* p = filterList;

    size_t len = 0;
    while (len < NFD_MAX_STRLEN - 1 && isalpha(*p)) {
        len = p - filterList;
        outExt[len] = *p;
        p++;
    }
    outExt[len + 1] = '\0';
}

static void
SetDefaultDir(GtkWidget* dialog, const char* defaultPath)
{
    fflush(stdout);
    if (!defaultPath || defaultPath[0] == '\0')
        return;

    /* GTK+ manual recommends not specifically setting the default path.
       We do it anyway in order to be consistent across platforms.

       If consistency with the native OS is preferred, this is the section
       to comment out. -ml */

    if (ftg_is_dir(defaultPath))
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), defaultPath);
    else
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), defaultPath);
}

static nfdresult_t
AllocPathSet(GSList* fileList, nfdpathset_t* pathSet)
{
    size_t     bufSize = 0;
    GSList*    node;
    nfdchar_t* p_buf;
    size_t     count = 0;

    assert(fileList);
    assert(pathSet);

    pathSet->count = (size_t)g_slist_length(fileList);
    assert(pathSet->count > 0);

    pathSet->indices = NFDi_Malloc(sizeof(size_t) * pathSet->count);
    if (!pathSet->indices) {
        return NFD_ERROR;
    }

    /* count the total space needed for buf */
    for (node = fileList; node; node = node->next) {
        assert(node->data);
        bufSize += strlen((const gchar*)node->data) + 1;
    }

    pathSet->buf = NFDi_Malloc(sizeof(nfdchar_t) * bufSize);

    /* fill buf */
    p_buf = pathSet->buf;
    for (node = fileList; node; node = node->next) {
        nfdchar_t* path = (nfdchar_t*)(node->data);
        size_t     byteLen = strlen(path) + 1;
        ptrdiff_t  index;

        memcpy(p_buf, path, byteLen);
        g_free(node->data);

        index = p_buf - pathSet->buf;
        assert(index >= 0);
        pathSet->indices[count] = (size_t)index;

        p_buf += byteLen;
        ++count;
    }

    g_slist_free(fileList);

    return NFD_OKAY;
}

static void
WaitForCleanup(void)
{
    while (gtk_events_pending()) gtk_main_iteration();
}


static char*
AllocUserFilename(GtkWidget* dialog, char* gtk_filename)
{
    // polyfill: if no extension, add the first extension from the
    // gtk file filter.
    char filter_ext[NFD_MAX_STRLEN] = {0};

    GtkFileFilter* ff = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
    if (ff) {
        GetFirstFilterExtension(gtk_file_filter_get_name(GTK_FILE_FILTER(ff)),
                                filter_ext);
    }

    char* specified_ext = ftg_get_filename_ext(gtk_filename);
    char* outPath;

    if (specified_ext[0] == '\0' && filter_ext[0] != '\0' &&
        gtk_filename[strlen(gtk_filename) - 1] != '.') {
        outPath = ftg_strcatall(3, gtk_filename, ".", filter_ext);
    } else {
        outPath = ftg_strcatall(1, gtk_filename);
    }

    return outPath;
}


static void
ConfigureFocus(GtkWidget* dialog)
{
#if defined(GDK_WINDOWING_X11)
    /* Work around focus issue on X11 (https://github.com/mlabbe/nativefiledialog/issues/79) */
    gtk_widget_show_all(dialog);
    if (GDK_IS_X11_DISPLAY(gdk_display_get_default())) {
        GdkWindow* window = gtk_widget_get_window(dialog);
        gdk_window_set_events(
            window, gdk_window_get_events(window) | GDK_PROPERTY_CHANGE_MASK);
        gtk_window_present_with_time(GTK_WINDOW(dialog),
                                     gdk_x11_get_server_time(window));
    }
#endif /* defined(GDK_WINDOWING_X11) */
}

/* public */

nfdresult_t
NFD_OpenDialog(const nfdchar_t* filterList, const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    GtkWidget*  dialog;
    nfdresult_t result;

    if (!gtk_init_check(NULL, NULL)) {
        NFDi_SetError(INIT_FAIL_MSG);
        return NFD_ERROR;
    }

    // TODO: use https://developer.gnome.org/gtk3/stable/gtk3-GtkFileChooserNative.html
    // when on gtk3!

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    /* Build the filter list */
    AddFiltersToDialog(dialog, filterList);

    /* Set the default dir */
    SetDefaultDir(dialog, defaultPath);

    ConfigureFocus(dialog);

    result = NFD_CANCEL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        *outPath = AllocUserFilename(dialog, filename);
        g_free(filename);

        if (!(*outPath)) {
            gtk_widget_destroy(dialog);
            NFDi_SetError("Error allocating bytes");
            return NFD_ERROR;
        }
        result = NFD_OKAY;
    }

    WaitForCleanup();
    gtk_widget_destroy(dialog);
    WaitForCleanup();

    return result;
}


nfdresult_t
NFD_OpenDialogMultiple(const nfdchar_t* filterList,
                       const nfdchar_t* defaultPath,
                       nfdpathset_t*    outPaths)
{
    GtkWidget*  dialog;
    nfdresult_t result;

    if (!gtk_init_check(NULL, NULL)) {
        NFDi_SetError(INIT_FAIL_MSG);
        return NFD_ERROR;
    }

    dialog = gtk_file_chooser_dialog_new("Open Files",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    /* Build the filter list */
    AddFiltersToDialog(dialog, filterList);

    /* Set the default dir */
    SetDefaultDir(dialog, defaultPath);

    ConfigureFocus(dialog);

    result = NFD_CANCEL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        GSList* fileList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        if (AllocPathSet(fileList, outPaths) == NFD_ERROR) {
            gtk_widget_destroy(dialog);
            return NFD_ERROR;
        }

        result = NFD_OKAY;
    }

    WaitForCleanup();
    gtk_widget_destroy(dialog);
    WaitForCleanup();

    return result;
}

nfdresult_t
NFD_SaveDialog(const nfdchar_t* filterList, const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    GtkWidget*  dialog;
    nfdresult_t result;

    if (!gtk_init_check(NULL, NULL)) {
        NFDi_SetError(INIT_FAIL_MSG);
        return NFD_ERROR;
    }

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Save",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    /* Build the filter list */
    AddFiltersToDialog(dialog, filterList);

    /* Set the default dir */
    SetDefaultDir(dialog, defaultPath);
    ConfigureFocus(dialog);

    result = NFD_CANCEL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        *outPath = AllocUserFilename(dialog, filename);
        g_free(filename);

        if (!(*outPath)) {
            gtk_widget_destroy(dialog);
            NFDi_SetError("Error allocating bytes");
            return NFD_ERROR;
        }

        result = NFD_OKAY;
    }

    WaitForCleanup();
    gtk_widget_destroy(dialog);
    WaitForCleanup();

    return result;
}

nfdresult_t
NFD_PickFolder(const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    GtkWidget*  dialog;
    nfdresult_t result;

    if (!gtk_init_check(NULL, NULL)) {
        NFDi_SetError(INIT_FAIL_MSG);
        return NFD_ERROR;
    }

    dialog = gtk_file_chooser_dialog_new("Select folder",
                                         NULL,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Select",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);


    /* Set the default dir */
    SetDefaultDir(dialog, defaultPath);

    ConfigureFocus(dialog);

    result = NFD_CANCEL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        *outPath = AllocUserFilename(dialog, filename);
        g_free(filename);

        if (!(*outPath)) {
            gtk_widget_destroy(dialog);
            NFDi_SetError("Error allocating bytes");
            return NFD_ERROR;
        }

        result = NFD_OKAY;
    }

    WaitForCleanup();
    gtk_widget_destroy(dialog);
    WaitForCleanup();

    return result;
}
