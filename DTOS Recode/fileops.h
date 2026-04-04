#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// delete_via_ofn - opens a file dialog (using internal ofn) and deletes the selected file.
// Returns: 1 if deleted, 0 if canceled or failed.
int delete_via_ofn(void);

#ifdef __cplusplus
}
#endif
