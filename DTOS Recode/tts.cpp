#include "tts.h"
#include <sapi.h>
#include <string>
#include <codecvt>
#include <locale>

static std::wstring utf8_to_wstring(const std::string &s) {
    if (s.empty()) return {};
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    std::wstring w(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], size_needed);
    return w;
}

int tts(const char *utf8text, int speed) {
    if (!utf8text) return 1;
    std::wstring w = utf8_to_wstring(std::string(utf8text));
    // map speed 1-5 to rate -10..10
    int rate = (speed - 3) * 5; // 1->-10, 3->0, 5->10

    ISpVoice *pVoice = NULL;
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return 2;
    hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if (SUCCEEDED(hr)) {
        pVoice->SetRate(rate);
        hr = pVoice->Speak(w.c_str(), SPF_IS_XML | SPF_ASYNC, NULL);
        // Wait for completion
        if (SUCCEEDED(hr)) {
            // Polling for state until done
            SPVOICESTATUS status;
            do {
                pVoice->GetStatus(&status, NULL);
                Sleep(50);
            } while (status.dwRunningState == SRSEIsSpeaking);
        }
        pVoice->Release();
    }
    CoUninitialize();
    return SUCCEEDED(hr) ? 0 : 3;
}
