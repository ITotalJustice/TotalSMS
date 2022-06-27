#include "ifile.h"
#include "../util.h"
#include <stdlib.h>


void ifile_close(IFile_t* ifile) {
    if (ifile) {
        if (ifile->close) {
            ifile->close(ifile->_private);
        }
        free(ifile);
    }
}

bool ifile_read(IFile_t* ifile, void* data, size_t len) {
    if (ifile->read) {
        return ifile->read(ifile->_private, data, len);
    }
    return false;
}

bool ifile_write(IFile_t* ifile, const void* data, size_t len) {
    if (ifile->write) {
        return ifile->write(ifile->_private, data, len);
    }
    return false;
}

bool ifile_seek(IFile_t* ifile, long offset, int whence) {
    if (ifile->seek) {
        return ifile->seek(ifile->_private, offset, whence);
    }
    return false;
}

size_t ifile_tell(IFile_t* ifile) {
    if (ifile->tell) {
        return ifile->tell(ifile->_private);
    }
    return 0;
}

size_t ifile_size(IFile_t* ifile) {
    if (ifile->size) {
        return ifile->size(ifile->_private);
    }
    return 0;
}
