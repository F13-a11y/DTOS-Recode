#pragma once
#include <string>
#include <windows.h>

// File filter components (English - not Chinese)
// These are wchar_t literals to be used when building the OPENFILENAME filter buffer.
// Each filter chunk must be single-null terminated. We'll append a final '\0' for double termination in code.
#define OFN_FILTER_TEXT   L"Text Files\0*.txt;*.log\0"
#define OFN_FILTER_IMAGES L"Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0"
#define OFN_FILTER_CSV    L"CSV Files\0*.csv\0"
#define OFN_FILTER_ALL    L"All Files\0*.*\0"
// System and configuration related file filters (English - separated)
// Keep these separate so the caller can choose whether to include them.
#define OFN_FILTER_SYS  L"System Files\0*.sys\0"
#define OFN_FILTER_CFG  L"Config Files\0*.cfg\0"
#define OFN_FILTER_JSON L"JSON Files\0*.json\0"
#define OFN_FILTER_DLL  L"DLL Files\0*.dll\0"

// ofn() - show an Open File dialog and return the selected file path as UTF-8 string.
// Returns an empty string if the user cancels or an error occurs.
// ofn(allowSystem = false): when allowSystem is false (default) the dialog
// will NOT include potentially sensitive extensions like .sys or .dll in the
// filter list. To allow those filters, call with allowSystem = true.
std::string ofn(bool allowSystem = false);

// ofn_scr() - show an Open File dialog filtered to .scr files (screensavers)
// Returns selected path as UTF-8 string or empty on cancel.
std::string ofn_scr();
// ofn_dtos() - show an Open File dialog filtered to .dtos files and return selected path as UTF-8
std::string ofn_dtos();
