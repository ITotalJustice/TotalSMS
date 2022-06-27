/*
  Native File Dialog

  http://www.frogtoss.com/labs
*/

// copied from: https://github.com/wheybags/simple_exec/blob/5a74c507c4ce1b2bb166177ead4cca7cfa23cb35/simple_exec.h


int runCommand(char** stdOut,
               int*   stdOutByteCount,
               int*   returnCode,
               int    includeStdErr,
               char*  command,
               ...);
int runCommandArray(char**       stdOut,
                    int*         stdOutByteCount,
                    int*         returnCode,
                    int          includeStdErr,
                    char* const* allArgs);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#define FTG_IMPLEMENT_CORE
#include "ftg_core.h"

#define release_assert(exp)                                                    \
    {                                                                          \
        if (!(exp)) {                                                          \
            abort();                                                           \
        }                                                                      \
    }

enum PIPE_FILE_DESCRIPTORS { READ_FD = 0, WRITE_FD = 1 };

enum RUN_COMMAND_ERROR { COMMAND_RAN_OK = 0, COMMAND_NOT_FOUND = 1 };

void
sigchldHandler(int p)
{
    FTG_UNUSED(p);
}

int
runCommandArray(char**       stdOut,
                int*         stdOutByteCount,
                int*         returnCode,
                int          includeStdErr,
                char* const* allArgs)
{
    // adapted from: https://stackoverflow.com/a/479103

    int  bufferSize = 256;
    char buffer[bufferSize + 1];

    int   dataReadFromChildDefaultSize = bufferSize * 5;
    int   dataReadFromChildSize = dataReadFromChildDefaultSize;
    int   dataReadFromChildUsed = 0;
    char* dataReadFromChild = (char*)malloc(dataReadFromChildSize);


    int parentToChild[2];
    release_assert(pipe(parentToChild) == 0);

    int childToParent[2];
    release_assert(pipe(childToParent) == 0);

    int errPipe[2];
    release_assert(pipe(errPipe) == 0);

    void (*prevHandler)(int);
    prevHandler = signal(SIGCHLD, sigchldHandler);

    pid_t pid;
    switch (pid = fork()) {
    case -1: {
        release_assert(0 && "Fork failed");
        break;
    }

    case 0:  // child
    {
        release_assert(dup2(parentToChild[READ_FD], STDIN_FILENO) != -1);
        release_assert(dup2(childToParent[WRITE_FD], STDOUT_FILENO) != -1);

        if (includeStdErr) {
            release_assert(dup2(childToParent[WRITE_FD], STDERR_FILENO) != -1);
        } else {
            int devNull = open("/dev/null", O_WRONLY);
            release_assert(dup2(devNull, STDERR_FILENO) != -1);
        }

        // unused
        release_assert(close(parentToChild[WRITE_FD]) == 0);
        release_assert(close(childToParent[READ_FD]) == 0);
        release_assert(close(errPipe[READ_FD]) == 0);

        const char* command = allArgs[0];
        execvp(command, allArgs);

        char    err = 1;
        ssize_t result = write(errPipe[WRITE_FD], &err, 1);
        release_assert(result != -1);

        close(errPipe[WRITE_FD]);
        close(parentToChild[READ_FD]);
        close(childToParent[WRITE_FD]);

        exit(0);
    }


    default:  // parent
    {
        // unused
        release_assert(close(parentToChild[READ_FD]) == 0);
        release_assert(close(childToParent[WRITE_FD]) == 0);
        release_assert(close(errPipe[WRITE_FD]) == 0);

        while (1) {
            ssize_t bytesRead = 0;
            switch (bytesRead = read(childToParent[READ_FD], buffer, bufferSize)) {
            case 0:  // End-of-File, or non-blocking read.
            {
                int status = 0;
                release_assert(waitpid(pid, &status, 0) == pid);

                // done with these now
                release_assert(close(parentToChild[WRITE_FD]) == 0);
                release_assert(close(childToParent[READ_FD]) == 0);

                char    errChar = 0;
                ssize_t result = read(errPipe[READ_FD], &errChar, 1);
                release_assert(result != -1);
                close(errPipe[READ_FD]);

                if (errChar) {
                    free(dataReadFromChild);
                    return COMMAND_NOT_FOUND;
                }

                // free any un-needed memory with realloc + add a null terminator for convenience
                dataReadFromChild =
                    (char*)realloc(dataReadFromChild, dataReadFromChildUsed + 1);
                dataReadFromChild[dataReadFromChildUsed] = '\0';

                if (stdOut != NULL)
                    *stdOut = dataReadFromChild;
                else
                    free(dataReadFromChild);

                if (stdOutByteCount != NULL)
                    *stdOutByteCount = dataReadFromChildUsed;
                if (returnCode != NULL)
                    *returnCode = WEXITSTATUS(status);

                return COMMAND_RAN_OK;
            }
            case -1: {
                release_assert(0 && "read() failed");
                break;
            }

            default: {
                if (dataReadFromChildUsed + bytesRead + 1 >= dataReadFromChildSize) {
                    dataReadFromChildSize += dataReadFromChildDefaultSize;
                    dataReadFromChild =
                        (char*)realloc(dataReadFromChild, dataReadFromChildSize);
                }

                memcpy(dataReadFromChild + dataReadFromChildUsed, buffer, bytesRead);
                dataReadFromChildUsed += bytesRead;
                break;
            }
            }
        }
    }
    }
    signal(SIGCHLD, prevHandler);
    return -69; // silence warning
}

int
runCommand(char** stdOut, int* stdOutByteCount, int* returnCode, int includeStdErr, char* command, ...)
{
    va_list vl;
    va_start(vl, command);

    char* currArg = NULL;

    int    allArgsInitialSize = 16;
    int    allArgsSize = allArgsInitialSize;
    char** allArgs = (char**)malloc(sizeof(char*) * allArgsSize);
    allArgs[0] = command;

    int i = 1;
    do {
        currArg = va_arg(vl, char*);
        allArgs[i] = currArg;

        i++;

        if (i >= allArgsSize) {
            allArgsSize += allArgsInitialSize;
            allArgs = (char**)realloc(allArgs, sizeof(char*) * allArgsSize);
        }

    } while (currArg != NULL);

    va_end(vl);

    int retval =
        runCommandArray(stdOut, stdOutByteCount, returnCode, includeStdErr, allArgs);
    free(allArgs);
    return retval;
}


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
#include "nfd.h"


const char NO_ZENITY_MSG[] = "zenity not installed";


static char* my_strdup(const char* s) {
    const size_t len = strlen(s) + 1;
    char* buf = (char*)malloc(len);
    memcpy(buf, s, len);
    return buf;
}

static void
AddTypeToFilterName(const char* typebuf, char* filterName, size_t bufsize)
{
    size_t len = strlen(filterName);
    if (len > 0)
        strncat(filterName, " *.", bufsize - len - 1);
    else
        strncat(filterName, "--file-filter=*.", bufsize - len - 1);

    len = strlen(filterName);
    strncat(filterName, typebuf, bufsize - len - 1);
}

static void
AddFiltersToCommandArgs(char** commandArgs, int commandArgsLen, const char* filterList)
{
    char        typebuf[NFD_MAX_STRLEN] = {0};
    const char* p_filterList = filterList;
    char*       p_typebuf = typebuf;
    char        filterName[NFD_MAX_STRLEN] = {0};
    int         i;

    if (!filterList || strlen(filterList) == 0)
        return;

    while (1) {
        if (NFDi_IsFilterSegmentChar(*p_filterList)) {
            char typebufWildcard[NFD_MAX_STRLEN + 3];
            /* add another type to the filter */
            assert(strlen(typebuf) > 0);
            assert(strlen(typebuf) < NFD_MAX_STRLEN - 1);

            snprintf(typebufWildcard, NFD_MAX_STRLEN + 3, "*.%s", typebuf);

            AddTypeToFilterName(typebuf, filterName, NFD_MAX_STRLEN);

            p_typebuf = typebuf;
            memset(typebuf, 0, sizeof(char) * NFD_MAX_STRLEN);
        }

        if (*p_filterList == ';' || *p_filterList == '\0') {
            /* end of filter -- add it to the dialog */

            for (i = 0; commandArgs[i] != NULL && i < commandArgsLen; i++)
                ;

            commandArgs[i] = my_strdup(filterName);

            filterName[0] = '\0';

            if (*p_filterList == '\0')
                break;
        }

        if (!NFDi_IsFilterSegmentChar(*p_filterList)) {
            *p_typebuf = *p_filterList;
            p_typebuf++;
        }

        p_filterList++;
    }

    /* always append a wildcard option to the end*/

    for (i = 0; commandArgs[i] != NULL && i < commandArgsLen; i++)
        ;

    commandArgs[i] = my_strdup("--file-filter=*.*");
}

static nfdresult_t
ZenityCommon(char**      command,
             int         commandLen,
             const char* defaultPath,
             const char* filterList,
             char**      stdOut)
{
    if (defaultPath != NULL) {
        const char* prefix = "--filename=";
        int   len = strlen(prefix) + strlen(defaultPath) + 1;

        char* tmp = (char*)calloc(len, 1);
        strcat(tmp, prefix);
        strcat(tmp, defaultPath);

        int i;
        for (i = 0; command[i] != NULL && i < commandLen; i++)
            ;

        command[i] = tmp;
    }

    AddFiltersToCommandArgs(command, commandLen, filterList);

    int byteCount = 0;
    int exitCode = 0;
    int processInvokeError =
        runCommandArray(stdOut, &byteCount, &exitCode, 0, command);

    for (int i = 0; command[i] != NULL && i < commandLen; i++) free(command[i]);

    nfdresult_t result = NFD_OKAY;

    if (processInvokeError == COMMAND_NOT_FOUND) {
        NFDi_SetError(NO_ZENITY_MSG);
        result = NFD_ERROR;
    }
    else {
        if (exitCode == 1) {
            result = NFD_CANCEL;
        }
    }

    return result;
}


static nfdresult_t
AllocPathSet(char* zenityList, nfdpathset_t* pathSet)
{
    assert(zenityList);
    assert(pathSet);

    size_t len = strlen(zenityList) + 1;
    pathSet->buf = NFDi_Malloc(len);

    int numEntries = 1;

    for (size_t i = 0; i < len; i++) {
        char ch = zenityList[i];

        if (ch == '|') {
            numEntries++;
            ch = '\0';
        }

        pathSet->buf[i] = ch;
    }

    pathSet->count = numEntries;
    assert(pathSet->count > 0);

    pathSet->indices = NFDi_Malloc(sizeof(size_t) * pathSet->count);

    int entry = 0;
    pathSet->indices[0] = 0;
    for (size_t i = 0; i < len; i++) {
        char ch = zenityList[i];

        if (ch == '|') {
            entry++;
            pathSet->indices[entry] = i + 1;
        }
    }

    return NFD_OKAY;
}

/* public */

nfdresult_t
NFD_OpenDialog(const char* filterList, const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    int   commandLen = 100;
    char* command[commandLen];
    memset(command, 0, commandLen * sizeof(char*));

    command[0] = my_strdup("zenity");
    command[1] = my_strdup("--file-selection");
    command[2] = my_strdup("--title=Open File");

    char*       stdOut = NULL;
    nfdresult_t result =
        ZenityCommon(command, commandLen, defaultPath, filterList, &stdOut);

    if (stdOut != NULL) {
        const size_t len = strlen(stdOut);
        if (len) {
            *outPath = NFDi_Malloc(len);
            memcpy(*outPath, stdOut, len);
            (*outPath)[len - 1] = '\0';  // trim out the final \n with a null terminator
        }

        free(stdOut);
    } else {
        *outPath = NULL;
    }

    return result;
}


nfdresult_t
NFD_OpenDialogMultiple(const nfdchar_t* filterList,
                       const nfdchar_t* defaultPath,
                       nfdpathset_t*    outPaths)
{
    int   commandLen = 100;
    char* command[commandLen];
    memset(command, 0, commandLen * sizeof(char*));

    command[0] = my_strdup("zenity");
    command[1] = my_strdup("--file-selection");
    command[2] = my_strdup("--title=Open Files");
    command[3] = my_strdup("--multiple");

    char*       stdOut = NULL;
    nfdresult_t result =
        ZenityCommon(command, commandLen, defaultPath, filterList, &stdOut);

    if (stdOut != NULL) {
        size_t len = strlen(stdOut);
        stdOut[len - 1] = '\0';  // remove trailing newline

        if (AllocPathSet(stdOut, outPaths) == NFD_ERROR)
            result = NFD_ERROR;

        free(stdOut);
    } else {
        result = NFD_ERROR;
    }

    return result;
}

nfdresult_t
NFD_SaveDialog(const nfdchar_t* filterList, const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    int   commandLen = 100;
    char* command[commandLen];
    memset(command, 0, commandLen * sizeof(char*));

    command[0] = my_strdup("zenity");
    command[1] = my_strdup("--file-selection");
    command[2] = my_strdup("--title=Save File");
    command[3] = my_strdup("--save");
    command[4] = my_strdup("--confirm-overwrite");

    char*       stdOut = NULL;
    nfdresult_t result =
        ZenityCommon(command, commandLen, defaultPath, filterList, &stdOut);

    if (stdOut != NULL) {
        size_t len = strlen(stdOut);
        *outPath = NFDi_Malloc(len);
        memcpy(*outPath, stdOut, len);
        (*outPath)[len - 1] = '\0';  // trim out the final \n with a null terminator
        free(stdOut);
    } else {
        *outPath = NULL;
    }

    return result;
}

nfdresult_t
NFD_PickFolder(const nfdchar_t* defaultPath, nfdchar_t** outPath)
{
    int   commandLen = 100;
    char* command[commandLen];
    memset(command, 0, commandLen * sizeof(char*));

    command[0] = my_strdup("zenity");
    command[1] = my_strdup("--file-selection");
    command[2] = my_strdup("--directory");
    command[3] = my_strdup("--title=Select folder");

    char*       stdOut = NULL;
    nfdresult_t result = ZenityCommon(command, commandLen, defaultPath, "", &stdOut);

    if (stdOut != NULL) {
        size_t len = strlen(stdOut);
        *outPath = NFDi_Malloc(len);
        memcpy(*outPath, stdOut, len);
        (*outPath)[len - 1] = '\0';  // trim out the final \n with a null terminator
        free(stdOut);
    } else {
        *outPath = NULL;
    }

    return result;
}
