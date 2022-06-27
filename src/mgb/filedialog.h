#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum FileDialogResultType {
	FileDialogResultType_OK,
	FileDialogResultType_ERROR,
	FileDialogResultType_CANCEL,
};

struct FileDialogResult {
	char path[0x301];
	enum FileDialogResultType type;
};

struct FileDialogResult filedialog_open_file(const char* filters);
struct FileDialogResult filedialog_open_folder(void);
struct FileDialogResult filedialog_save_file(const char* filters);

#ifdef __cplusplus
}
#endif
