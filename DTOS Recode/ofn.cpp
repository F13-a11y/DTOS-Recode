#include "ofn.h"
#include <commdlg.h>
#include <sstream>
#include <locale>
#include <codecvt>
#include <vector>
#include <utility>

// Helper: convert wide string to UTF-8 std::string
static std::string wtos(const std::wstring &ws) {
    if (ws.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string ofn_dtos() {
    wchar_t filename[MAX_PATH] = {0};
    // Filter: .dtos files only
    const wchar_t filter[] = L"DTOS Files\0*.dtos\0All Files\0*.*\0\0";
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    BOOL res = GetOpenFileNameW(&ofn);
    if (!res) return std::string();
    return wtos(std::wstring(filename));
}

std::string ofn_scr() {
    wchar_t filename[MAX_PATH] = {0};
    // Filter: Screensavers (.scr) only
    const wchar_t filter[] = L"Screensaver Files\0*.scr\0All Files\0*.*\0\0";
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    BOOL res = GetOpenFileNameW(&ofn);
    if (!res) return std::string();
    return wtos(std::wstring(filename));
}

std::string ofn(bool allowSystem) {
    wchar_t filename[MAX_PATH] = {0};
#pragma region OFNFILTERS
    // Build filter buffer explicitly as pairs: display\0pattern\0 ... final \0
    std::vector<std::pair<std::wstring, std::wstring>> chunks;
    chunks.emplace_back(L"Text Files", L"*.txt;*.log");
    chunks.emplace_back(L"Image Files", L"*.png;*.jpg;*.jpeg;*.bmp");
    chunks.emplace_back(L"CSV Files", L"*.csv");
    if (allowSystem) {
        chunks.emplace_back(L"System Files", L"*.sys");
        chunks.emplace_back(L"Config Files", L"*.cfg");
        chunks.emplace_back(L"JSON Files", L"*.json");
        chunks.emplace_back(L"DLL Files", L"*.dll");
    }
    chunks.emplace_back(L"All Files", L"*.*");
#pragma endregion
    std::vector<wchar_t> filterbuf;
    for (auto &p : chunks) {
        filterbuf.insert(filterbuf.end(), p.first.begin(), p.first.end());
        filterbuf.push_back(L'\0');
        filterbuf.insert(filterbuf.end(), p.second.begin(), p.second.end());
        filterbuf.push_back(L'\0');
    }
    // final double null
    filterbuf.push_back(L'\0');

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filterbuf.empty() ? NULL : filterbuf.data();
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    HWND owner = GetActiveWindow();
    BOOL res = GetOpenFileNameW(&ofn);
    if (!res) return std::string();
    return wtos(std::wstring(filename));
}
