#include "filedialog.h"


#ifndef HAS_NFD
	#define HAS_NFD 0
#endif

#if HAS_NFD

#include <nfd.h>
#include <string.h>
#include <stdlib.h>


struct FileDialogResult filedialog_open_file(const char* filters) {
	struct FileDialogResult result = {0};
	nfdchar_t* nfd_out_path = NULL;

	switch (NFD_OpenDialog(filters, NULL, &nfd_out_path)) {
	    case NFD_ERROR:
	    	result.type = FileDialogResultType_ERROR;
	    	break;

	    case NFD_OKAY:
	    	strncpy(result.path, nfd_out_path, sizeof(result.path) - 1);
	    	result.type = FileDialogResultType_OK;
	    	break;

	    case NFD_CANCEL:
	    	result.type = FileDialogResultType_CANCEL;
	    	break;
	}

	if (nfd_out_path) {
		free(nfd_out_path);
	}

	return result;
}

struct FileDialogResult filedialog_open_folder(void) {
	struct FileDialogResult result = {0};
	nfdchar_t* nfd_out_path = NULL;

	switch (NFD_PickFolder(NULL, &nfd_out_path)) {
	    case NFD_ERROR:
	    	result.type = FileDialogResultType_ERROR;
	    	break;

	    case NFD_OKAY: {
	    	strncpy(result.path, nfd_out_path, sizeof(result.path) - 1);
	    	result.type = FileDialogResultType_OK;
	    }	break;

	    case NFD_CANCEL:
	    	result.type = FileDialogResultType_CANCEL;
	    	break;
	}

	if (nfd_out_path) {
		free(nfd_out_path);
	}

	return result;
}

struct FileDialogResult filedialog_save_file(const char* filters) {
	struct FileDialogResult result = {0};
	nfdchar_t* nfd_out_path = NULL;

	switch (NFD_SaveDialog(filters, NULL, &nfd_out_path)) {
	    case NFD_ERROR:
	    	result.type = FileDialogResultType_ERROR;
	    	break;

	    case NFD_OKAY:
	    	strncpy(result.path, nfd_out_path, sizeof(result.path) - 1);
	    	result.type = FileDialogResultType_OK;
	    	break;

	    case NFD_CANCEL:
	    	result.type = FileDialogResultType_CANCEL;
	    	break;
	}

	if (nfd_out_path) {
		free(nfd_out_path);
	}

	return result;
}

#else

// these are stubbed...
struct FileDialogResult filedialog_open_file(const char* filters) {
	(void)filters;

	struct FileDialogResult r;
	r.type = FileDialogResultType_ERROR;
	return r;
}

struct FileDialogResult filedialog_open_folder(void) {
	struct FileDialogResult r;
	r.type = FileDialogResultType_ERROR;
	return r;
}

struct FileDialogResult filedialog_save_file(const char* filters) {
	(void)filters;

	struct FileDialogResult r;
	r.type = FileDialogResultType_ERROR;
	return r;
}

#endif
