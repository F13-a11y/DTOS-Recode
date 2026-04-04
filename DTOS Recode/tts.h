#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// tts: speak text using Windows SAPI. speed 1-5 (1 slow ... 5 fast)
// Returns 0 on success, non-zero on failure.
int tts(const char *utf8text, int speed);

#ifdef __cplusplus
}
#endif
