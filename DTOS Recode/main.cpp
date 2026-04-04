#include "includeset.h"
#include "color.h"
#include "about.h"
#include "ofn.h"
#include "fileops.h"
#include "tts.h"
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
// aboutex removed per user request
using namespace std;
int main() {
    // Enable ANSI escape sequence processing on Windows consoles
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    // Capture default console attributes so we can reset later
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    WORD defaultAttr = 7;
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        defaultAttr = csbi.wAttributes;
    }

    auto splitArgs = [](const string &line) {
        vector<string> out;
        istringstream iss(line);
        string token;
        while (iss >> std::quoted(token) || iss >> token) {
            out.push_back(token);
        }
        return out;
    };

    auto printHelp = [&]() {
        cout << ANSI_BYELLOW << "Simple Terminal - commands:" << ANSI_RESET << '\n';
        cout << ANSI_BYELLOW << "Note: wrap multi-word arguments in double quotes, e.g. talk \"hello world\" 3" << ANSI_RESET << '\n';
        cout << "  help        - show this message" << '\n';
        cout << "  cls         - clear screen" << '\n';
        cout << "  cdview      - toggle showing full path in prompt" << '\n';
        cout << "  rundll      - run a DLL file" << '\n';
        cout << "  del         - delete a file (opens file dialog)" << '\n';
        cout << "  colorset    - set console colors using two hex digits (e.g. 1A) or 'reset'" << '\n';
        cout << "  start       - open a file with the system default application" << '\n';
        cout << "  talk        - text-to-speech: talk \"text\" <1-5 speed>" << '\n';
        cout << "  echo        - print text" << '\n';
        cout << "  about       - show about info" << '\n';
        cout << "  scrsvr      - list/run system screensavers or use 'scrsvr spec' to pick a custom .scr" << '\n';
        cout << "  xorenc      - XOR-encrypt text: xorenc \"text\" \"key\" -> outputs hex" << '\n';
        cout << "  xordec      - XOR-decrypt hex: xordec \"hex\" \"key\" -> outputs text" << '\n';
        cout << "  dev         - open author link in default browser" << '\n';

    };

    cout << ANSI_BGREEN << "DTOS Recode  (" << VER << ")" << ANSI_RESET << '\n';
    cout << "Type 'help' for commands.\n";

    filesystem::path cwd = filesystem::current_path();
    bool cdView = false; // when true, prompt shows full path; otherwise shows !~.
    string line;
    while (true) {
        // Prompt
        if (cdView) cout << ANSI_BBLUE << cwd.string() << " > " << ANSI_RESET;
        else cout << ANSI_BGREEN << "!~." << " > " << ANSI_RESET;
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        auto args = splitArgs(line);
        if (args.empty()) continue;
        string cmd = args[0];
        // to lower
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "help") {
            printHelp();
        } else if (cmd == "cls" || cmd == "clear") {
            // ANSI clear
            cout << "\x1b[2J\x1b[H";
        } else if (cmd == "cdview") {
            cdView = !cdView;
            cout << (cdView ? "cdview: ON\n" : "cdview: OFF\n");
        } else if (cmd == "rundll") {
            // Ask user to pick a DLL via the ofn dialog. Do not include system filters by default.
            string path = ofn(false);
            if (path.empty()) { cout << "rundll: canceled\n"; }
            else {
                // Warn the user that running DLLs may be dangerous
                MessageBoxA(NULL, "Warning: Running arbitrary DLLs can be dangerous. This program is not responsible for any harm.", "Warning", MB_OK | MB_ICONWARNING);
                // Build command line for rundll32: rundll32.exe "<path>",#1
                string cmdline = "rundll32.exe \"" + path + "\",#1";
                // Launch using CreateProcess
                STARTUPINFOA si = {0}; PROCESS_INFORMATION pi = {0}; si.cb = sizeof(si);
                BOOL ok = CreateProcessA(NULL, &cmdline[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                if (!ok) {
                    cout << "rundll: failed to start rundll32\n";
                } else {
                    cout << "rundll: started rundll32 (pid=" << pi.dwProcessId << ")\n";
                    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
                }
            }
        } else if (cmd == "del") {
            // Call C-linkage helper to open ofn and delete selected file
            int res = delete_via_ofn();
            if (res == 1) cout << "del: file deleted\n";
            else if (res == 0) cout << "del: canceled or failed\n";
        } else if (cmd == "colorset") {
            if (args.size() < 2) { cout << "colorset: missing two hex digits (BG)\n"; }
            else {
                string v = args[1];
                if (v == "reset" || v == "default") {
                    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                    SetConsoleTextAttribute(h, defaultAttr);
                    cout << "colorset: reset to default\n";
                } else if (v.size() != 2) { cout << "colorset: expected exactly two hex digits (e.g. 1A) or 'reset'\n"; }
                else {
                    auto hexval = [](char c)->int {
                        if (c >= '0' && c <= '9') return c - '0';
                        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                        return -1;
                    };
                    int hi = hexval(v[0]); int lo = hexval(v[1]);
                    if (hi < 0 || lo < 0) { cout << "colorset: invalid hex digits\n"; }
                    else {
                        WORD attr = (WORD)((hi << 4) | (lo & 0xF));
                        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                        SetConsoleTextAttribute(h, attr);
                        cout << "colorset: applied\n";
                    }
                }
            }
        } else if (cmd == "start") {
            // Open a file with default handler (like Windows 'start' in batch)
            string path = ofn(false);
            if (path.empty()) { cout << "start: canceled\n"; }
            else {
                // Use ShellExecuteA to open with associated application
                HINSTANCE r = ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                if ((INT_PTR)r <= 32) cout << "start: failed to open file\n";
                else cout << "start: opened\n";
            }
        } else if (cmd == "talk") {
            // talk "text" <1-5 speed>
            if (args.size() < 2) { cout << "talk: missing text in quotes\n"; }
            else {
                string text = args[1];
                int speed = 3;
                if (args.size() >= 3) {
                    try { speed = stoi(args[2]); } catch(...) { speed = 3; }
                }
                // clamp speed 1-5
                if (speed < 1) speed = 1; if (speed > 5) speed = 5;
                int res = tts(text.c_str(), speed);
                if (res != 0) cout << "talk: failed to speak\n";
            }
        } else if (cmd == "echo") {
            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) cout << ' ';
                cout << args[i];
            }
            cout << '\n';
        } else if (cmd == "about") {
            cout << ANSI_BYELLOW << "Version: " << VER << "\nAuthor: " << AUTHOR << ANSI_RESET << '\n';
        } else if (cmd == "dev") {
            // open AUTHOR link in default browser
            HINSTANCE r = ShellExecuteA(NULL, "open", AUTHOR, NULL, NULL, SW_SHOWNORMAL);
            if ((INT_PTR)r <= 32) cout << "dev: failed to open link\n";
            else cout << "dev: opened " << AUTHOR << "\n";
        } else if (cmd == "xorenc") {
            if (args.size() < 3) { cout << "xorenc: usage: xorenc \"text\" \"key\"\n"; }
            else {
                string text = args[1];
                string key = args[2];
                if (key.empty()) { cout << "xorenc: key cannot be empty\n"; }
                else {
                    string out; out.reserve(text.size());
                    for (size_t i = 0; i < text.size(); ++i) out.push_back(text[i] ^ key[i % key.size()]);
                    // output as hex
                    std::ostringstream oss;
                    oss << std::hex << std::setfill('0');
                    for (unsigned char c : out) oss << std::setw(2) << (int)c;
                    cout << oss.str() << '\n';
                }
            }
        } else if (cmd == "xordec") {
            if (args.size() < 3) { cout << "xordec: usage: xordec \"hex\" \"key\"\n"; }
            else {
                string hex = args[1];
                string key = args[2];
                if (key.empty()) { cout << "xordec: key cannot be empty\n"; }
                else {
                    // normalize hex (allow upper/lower, optional spaces)
                    string h; h.reserve(hex.size());
                    for (char c : hex) if (!isspace((unsigned char)c)) h.push_back(c);
                    if (h.size() % 2 != 0) { cout << "xordec: invalid hex length\n"; }
                    else {
                        string bytes; bytes.reserve(h.size()/2);
                        try {
                            for (size_t i = 0; i < h.size(); i += 2) {
                                string byte = h.substr(i,2);
                                unsigned int val = 0;
                                std::stringstream ss; ss << std::hex << byte;
                                ss >> val;
                                bytes.push_back((char)val);
                            }
                        } catch(...) { cout << "xordec: invalid hex\n"; bytes.clear(); }
                        if (!bytes.empty()) {
                            string out; out.reserve(bytes.size());
                            for (size_t i = 0; i < bytes.size(); ++i) out.push_back(bytes[i] ^ key[i % key.size()]);
                            cout << out << '\n';
                        }
                    }
                }
            }
        } else if (cmd == "scrsvr") {
            // scrsvr [spec]
            if (args.size() >= 2 && args[1] == "spec") {
                string path = ofn_scr();
                if (path.empty()) { cout << "scrsvr: canceled\n"; }
                else {
                    cout << "scrsvr: running custom screensaver: " << path << '\n';
                    // Convert UTF-8 path to wide
                    int needed = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
                    std::wstring wpath(needed ? needed - 1 : 0, L'\0');
                    if (needed) MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], needed);
                    std::wstring cmdline = L"\"" + wpath + L"\" /s";
                    STARTUPINFOW si = {0}; PROCESS_INFORMATION pi = {0}; si.cb = sizeof(si);
                    BOOL ok = CreateProcessW(NULL, &cmdline[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                    if (!ok) cout << "scrsvr: failed to start screensaver\n";
                    else { cout << "scrsvr: started (pid=" << pi.dwProcessId << ")\n"; CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
                }
            } else {
                // List system .scr files from system directory
                WCHAR sysdir[MAX_PATH] = {0};
                if (!GetSystemDirectoryW(sysdir, MAX_PATH)) {
                    cout << "scrsvr: unable to get system directory\n";
                } else {
                    filesystem::path p(sysdir);
                    vector<filesystem::path> scrs;
                    try {
                        for (auto &entry : filesystem::directory_iterator(p)) {
                            if (!entry.is_regular_file()) continue;
                            auto ext = entry.path().extension().wstring();
                            for (auto &c : ext) c = towlower(c);
                            if (ext == L".scr") scrs.push_back(entry.path());
                        }
                    } catch (...) {}

                    if (scrs.empty()) {
                        cout << "scrsvr: no screensavers found in system directory\n";
                    } else {
                        cout << "Available system screensavers:\n";
                        for (size_t i = 0; i < scrs.size(); ++i) {
                            cout << "  [" << (i+1) << "] " << scrs[i].filename().string();
                            // also print friendly name without extension
                            cout << "\n";
                        }
                        cout << "Select number to run (0 to cancel): ";
                        string sel;
                        if (!getline(cin, sel)) sel = "0";
                        int idx = 0; try { idx = stoi(sel); } catch(...) { idx = 0; }
                        if (idx <= 0 || (size_t)idx > scrs.size()) { cout << "scrsvr: canceled\n"; }
                        else {
                            auto chosen = scrs[idx-1];
                            cout << "scrsvr: running " << chosen.filename().string() << "\n";
                            // run with /s using wide CreateProcess
                            std::wstring wpath = chosen.wstring();
                            std::wstring cmdline = L"\"" + wpath + L"\" /s";
                            STARTUPINFOW si = {0}; PROCESS_INFORMATION pi = {0}; si.cb = sizeof(si);
                            BOOL ok = CreateProcessW(NULL, &cmdline[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                            if (!ok) cout << "scrsvr: failed to start screensaver\n";
                            else { cout << "scrsvr: started (pid=" << pi.dwProcessId << ")\n"; CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
                        }
                    }
                }
            }
        } else {
            cout << "Unknown command: " << cmd << " (type help)\n";
        }
    }

    cout << "Goodbye.\n";
    return 0;
}
