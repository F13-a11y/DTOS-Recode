#include <windows.h>
#include <stdio.h>
#include "sysinfoembedded.h"

// RtlGetVersion function pointer
typedef LONG(WINAPI* RtlGetVersionPtr)(OSVERSIONINFOEXW*);

LONG RtlGetVersion(OSVERSIONINFOEXW* os) {
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr f = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (f != NULL) {
            return f(os);
        }
    }
    return -1;
}

unsigned long long get_ram_info() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullTotalPhys / (1024 * 1024); // MB
}

const char* get_win_version(DWORD build) {
    if (build >= 22000) {
        return "Windows 11";
    }
    else if (build >= 10240) {
        return "Windows 10";
    }
    return "Older Windows version";
}

void print_sysinfo() {
    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    DWORD build = 0;
    if (RtlGetVersion(&osvi) == 0) {
        build = osvi.dwBuildNumber;
    }

    printf("Total RAM: %llu MB\n", get_ram_info());
    printf("Operating System: %s (Build %lu)\n", get_win_version(build), build);
}
