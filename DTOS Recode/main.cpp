#include <signal.h>
#include "includeset.h"
#include "color.h"
#include "about.h"
#include "ofn.h"
#include "fileops.h"
#include "tts.h"
#include "sysinfoembedded.h"
#include "windowhandler.h"
#include "scriptparser.h"
#include "format.h"

void kaboom();

void kaboom() { // STACK OVERFLOW
	kaboom();
}

using namespace std;



int main(int argc, char** argv) {


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

    // Use the parser module we just added
    auto run_dtos_file = [&](const string &path) {
        if (path.empty()) { cout << "execute: canceled\n"; return; }
        cout << "execute: running automation file: " << path << "\n";
        vector<DtosJob> jobs;
        string err;
        if (!parse_dtos_file(path, jobs, err)) { cout << "execute: failed to parse file: " << err << "\n"; return; }

        long long totalLoops = 0;
        for (auto &j : jobs) totalLoops += j.loops;

        bool doExecute = true;
        // Detect nested execute usage
        bool nestedExecute = false;
        for (auto &j : jobs) {
            auto sa = splitArgs(j.command);
            if (!sa.empty()) {
                string t0 = sa[0]; transform(t0.begin(), t0.end(), t0.begin(), ::tolower);
                if (t0 == "execute") { nestedExecute = true; break; }
            }
        }
        if (nestedExecute) {
            std::ostringstream ms2;
            ms2 << "Automation file contains an 'execute' command which may open another .dtos script.\n"
                << "Chaining automation files can be dangerous.\n\nRun anyway?";
            int mb2 = MessageBoxA(NULL, ms2.str().c_str(), "DTOS Recode - Automation Warning", MB_YESNO | MB_ICONWARNING);
            if (mb2 != IDYES) { cout << "execute: canceled by user (nested execute)\n"; doExecute = false; }
        }
        if (doExecute && totalLoops > 15) {
            std::ostringstream ms;
            ms << "Automation file requests " << totalLoops << " total loop iterations (>15).\n"
               << "This may be malicious.\n\nRun the automation file?";
            int mb = MessageBoxA(NULL, ms.str().c_str(), "DTOS Recode - Automation Warning", MB_YESNO | MB_ICONWARNING);
            if (mb != IDYES) { cout << "execute: canceled by user\n"; doExecute = false; }
        }
        if (!doExecute) return;

        for (auto &j : jobs) {
            for (int rep = 0; rep < j.loops; ++rep) {
                auto subargs = splitArgs(j.command);
                if (subargs.empty()) continue;
                string subcmd = subargs[0]; transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);
                if (subcmd == "talk") {
                    if (subargs.size() < 2) { cout << "execute: talk missing text\n"; continue; }
                    string text = subargs[1]; int speed = 3;
                    if (subargs.size() >= 3) { try { speed = stoi(subargs[2]); } catch(...) { speed = 3; } }
                    if (speed < 1) speed = 1; if (speed > 5) speed = 5;
                    tts(text.c_str(), speed);
                } else if (subcmd == "echo") {
                    for (size_t k = 1; k < subargs.size(); ++k) { if (k>1) cout << ' '; cout << subargs[k]; }
                    cout << '\n';
                } else if (subcmd == "sleep") {
                    if (subargs.size() >= 2) {
                        int ms = 0; try { ms = stoi(subargs[1]); } catch(...) { ms = 0; }
                        if (ms > 0) Sleep(ms);
                    }
                } else {
                    string combined;
                    for (size_t k = 0; k < subargs.size(); ++k) { if (k) combined += ' '; combined += subargs[k]; }
                    string shellcmd = string("cmd /C ") + combined;
                    system(shellcmd.c_str());
                }
            }
        }
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
        cout << "  ping        - ping a host (uses system ping)" << '\n';
        cout << "  execute     - open a .dtos automation file and execute its commands" << '\n';
		cout << "  history     - show command history for this session" << '\n';
		cout << ANSI_RED << "  urlmon" << ANSI_RESET << "      - download a file from its file link "<< ANSI_YELLOW << "(Coming Soon)" << ANSI_RESET << '\n';
		cout << "  sysinfo     - show basic system information" << '\n';
		cout << "  sysinfoex   - show system information using system command" << '\n';
		cout << "  window      - open a simple window for debugging, testing DLLs etc." << '\n';
		cout << "  abort       - exit the program\n";
		cout << "  crash       - intentionally crash the program" << ANSI_RED << " (Debug and Reverse Engineering Only)" << ANSI_RESET << '\n';
		cout << "  format      - translate number formats\n";
		cout << ANSI_BYELLOW << "Automation file format (.dtos):" << ANSI_RESET << '\n';
		cout << ANSI_BLUE << "Read DTOSRULES.md for automation file syntax and security notes.\n" << ANSI_RESET;
    };
	cout << "Build Date: " << ANSI_BLUE << __DATE__ << " " << __TIME__ << ANSI_RESET << '\n';
	cout << "Timestamp View : " << ANSI_BLUE << __TIMESTAMP__ << ANSI_RESET << '\n';
    cout << ANSI_BGREEN << "DTOS Recode  (" << VER << ")" << ANSI_RESET << '\n';
    cout << "Type 'help' for commands.\n";
    filesystem::path cwd = filesystem::current_path();
    bool cdView = false; // when true, prompt shows full path; otherwise shows !~.
    // simple in-memory history of entered commands
    vector<string> history;
    string line;
    while (true) {
        // Prompt
        if (cdView) cout << ANSI_BBLUE << cwd.string() << " > " << ANSI_RESET;
        else cout << ANSI_BGREEN << "!~." << " > " << ANSI_RESET;
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        // store non-empty input into history
        history.push_back(line);
        auto args = splitArgs(line);
        if (args.empty()) continue;
        string cmd = args[0];
        // to lower
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "help") {
            printHelp();
        } else if (cmd == "history") {
            // print simple command history
            cout << "History (most recent last):\n";
            for (size_t i = 0; i < history.size(); ++i) cout << "  " << (i+1) << ": " << history[i] << '\n';
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
        } else if (cmd == "ping") {
            if (args.size() < 2) { cout << "ping: usage: ping <host> [options]\n"; }
            else {
                // rebuild the rest of the arguments into a single command
                std::string rest;
                for (size_t i = 1; i < args.size(); ++i) {
                    if (i > 1) rest += ' ';
                    rest += args[i];
                }
                std::string full = std::string("ping ") + rest;
                // use system() to run Windows ping
                int rcode = system(full.c_str());
                if (rcode != 0) cout << "ping: command finished with code " << rcode << "\n";
            }
        }

#pragma region PARSER


        else if (cmd == "execute") {
            // execute: open a .dtos file and run its commands
            string path = ofn_dtos();
            if (path.empty()) { cout << "execute: canceled\n"; }
            else {
                cout << "execute: running automation file: " << path << "\n";
                std::ifstream ifs(path);
                if (!ifs) { cout << "execute: failed to open file\n"; }
                else {
                    // Parse the file first into jobs (command string + loop count).
                    // This lets us compute total loop iterations before executing
                    // and warn the user if the script requests many iterations.
                    vector<pair<string,int>> jobs;
                    string fileline;
                    auto trim = [](string &s) {
                        size_t b = 0; while (b < s.size() && isspace((unsigned char)s[b])) ++b;
                        size_t e = s.size(); while (e > b && isspace((unsigned char)s[e-1])) --e;
                        s = s.substr(b, e-b);
                    };

                    while (std::getline(ifs, fileline)) {
                        string l = fileline;
                        if (!l.empty() && (unsigned char)l[0] == 0xEF) {
                            if (l.size() >= 3 && (unsigned char)l[1] == 0xBB && (unsigned char)l[2] == 0xBF) l = l.substr(3);
                        }
                        trim(l);
                        if (l.empty()) continue;
                        if (l.size() >= 1 && (l[0] == '#' || l[0] == ';')) continue;
                        string prefix = "run ";
                        string lower = l;
                        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                        if (lower.rfind(prefix, 0) != 0) { cout << "execute: skipping unsupported line: " << l << "\n"; continue; }
                        string payload = l.substr(prefix.size());
                        string cmdpart = payload;
                        int loops = 1;
                        size_t comma = payload.find(',');
                        if (comma != string::npos) {
                            cmdpart = payload.substr(0, comma);
                            string opt = payload.substr(comma + 1);
                            trim(cmdpart); trim(opt);
                            string optlow = opt; transform(optlow.begin(), optlow.end(), optlow.begin(), ::tolower);
                            if (optlow.rfind("loop", 0) == 0) {
                                string num = opt.substr(4);
                                trim(num);
                                try { loops = stoi(num); } catch(...) { loops = 1; }
                                if (loops < 1) loops = 1;
                            }
                        } else {
                            trim(cmdpart);
                        }
                        jobs.emplace_back(cmdpart, loops);
                    }

                    long long totalLoops = 0;
                    for (auto &j : jobs) totalLoops += j.second;
                    // Execution control flag: whether to run the parsed jobs.
                    bool doExecute = true;
                    // Detect if any job attempts to call 'execute' (which would open
                    // another .dtos). Treat this as potentially dangerous and warn.
                    bool nestedExecute = false;
                    for (auto &j : jobs) {
                        auto tmp = splitArgs(j.first);
                        if (!tmp.empty()) {
                            string t0 = tmp[0]; transform(t0.begin(), t0.end(), t0.begin(), ::tolower);
                            if (t0 == "execute") { nestedExecute = true; break; }
                        }
                    }
                    if (nestedExecute) {
                        std::ostringstream ms2;
                        ms2 << "Automation file contains an 'execute' command which may open another .dtos script.\n"
                            << "Chaining automation files can be dangerous.\n\nRun anyway?";
                        int mb2 = MessageBoxA(NULL, ms2.str().c_str(), "DTOS Recode - Automation Warning", MB_YESNO | MB_ICONWARNING);
                        if (mb2 != IDYES) {
                            cout << "execute: canceled by user (nested execute)\n";
                            doExecute = false;
                        }
                    }
                    if (totalLoops > 15) {
                        // Security check: warn the user when the automation file requests
                        // a large number of total loop iterations. This helps detect
                        // potentially malicious or runaway scripts before execution.
                        // Show a MessageBox with Yes/No; if the user selects No, cancel.
                        std::ostringstream ms;
                        ms << "Automation file requests " << totalLoops << " total loop iterations (>15).\n"
                           << "This may be malicious.\n\nRun the automation file?";
                        int mb = MessageBoxA(NULL, ms.str().c_str(), "DTOS Recode - Automation Warning", MB_YESNO | MB_ICONWARNING);
                        if (mb != IDYES) {
                            cout << "execute: canceled by user\n";
                            doExecute = false;
                        }
                    }

                    if (doExecute) {
                        for (auto &j : jobs) {
                            for (int rep = 0; rep < j.second; ++rep) {
                                auto subargs = splitArgs(j.first);
                                if (subargs.empty()) continue;
                                string subcmd = subargs[0]; transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);
                                if (subcmd == "talk") {
                                    if (subargs.size() < 2) { cout << "execute: talk missing text\n"; continue; }
                                    string text = subargs[1]; int speed = 3;
                                    if (subargs.size() >= 3) { try { speed = stoi(subargs[2]); } catch(...) { speed = 3; } }
                                    if (speed < 1) speed = 1; if (speed > 5) speed = 5;
                                    tts(text.c_str(), speed);
                                // after each built-in command we may allow a brief sleep
                                // if the command line included a 'sleep' token as: sleep <ms>
                                // however sleep is implemented as its own run entry below.
                                } else if (subcmd == "echo") {
                                    for (size_t k = 1; k < subargs.size(); ++k) { if (k>1) cout << ' '; cout << subargs[k]; }
                                    cout << '\n';
                                } else {
                                    string combined;
                                    for (size_t k = 0; k < subargs.size(); ++k) {
                                        if (k) combined += ' ';
                                        combined += subargs[k];
                                    }
                            // Support a standalone 'sleep <ms>' command inside .dtos
                            if (subcmd == "sleep") {
                                if (subargs.size() >= 2) {
                                    int ms = 0; try { ms = stoi(subargs[1]); } catch(...) { ms = 0; }
                                    if (ms > 0) Sleep(ms);
                                }
                            }
                                    string shellcmd = string("cmd /C ") + combined;
                                    system(shellcmd.c_str());
                                }
                            }
                        }
                    }
                }
            }

#pragma endregion

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
		}   // EASTER EGG FOR WHO JUST HAVES THE EXECUTABLE,NOT THE SOURCE CODE
        else if (cmd == "rickroll")
        {
            ShellExecuteA(NULL, "open", "https://www.youtube.com/watch?v=dQw4w9WgXcQ", NULL, NULL, SW_SHOWNORMAL);
        }
        else if (cmd == "sysinfo") {
			cout << ANSI_BYELLOW << "System Information:" << ANSI_RESET << '\n';
			print_sysinfo();
       
            }
        else if (cmd == "sysinfoex") {
            cout << ANSI_BYELLOW << "Detailed System Information:" << ANSI_RESET << '\n';
            system("systeminfo");
		}
        else if (cmd == "urlmon") {
			MessageBoxA(NULL, "The 'urlmon' command is coming soon! This will allow downloading files from URLs directly in the terminal.", "Coming Soon", MB_OK | MB_ICONINFORMATION);
		}
		else if (cmd == "window") {
			cout << "Opening a simple window. Close it to return to the terminal.\n";
            HINSTANCE hInst = GetModuleHandle(NULL);
            WindowHandler(hInst, L"TEST");
		}
        else if (cmd == "abort") {
            cout << "Chose Abort Type\n1. Force Shutdown\n2. Normal Shutdown (w/o main function return)\n3. Custom Return Value" << ANSI_RED << " (Debug and Reverse Engineering Only)" << ANSI_RESET << "\n4. Slient\n5. Cancel\n" << ANSI_RED << "BE CAREFUL WHEN USING 'Custom Return Value' EXPECTION HANDLING NOT IMPLEMENTED" << ANSI_RESET << "\n";
            string choice;
            if (getline(cin, choice)) {
                if (choice == "1") {
                    TerminateProcess(GetCurrentProcess(), 1); // direkt process kill
                }
                else if (choice == "2") {
                    ExitProcess(0);
                }
                else if (choice == "3") {
                    int abortcode;
                    cout << "Enter custom exit code (integer): ";
                    cin >> abortcode;
                    return abortcode;
                }
                else if (choice == "4") {
                    FreeConsole();
                    ExitProcess(0);
                }
                else if (choice == "5") {
                    cout << "Exit canceled.\n";
                }
            }
            else {
                cout << "No input received. Exit canceled.\n";
            }
        }

		else if (cmd == "crash") {
            int crashChoice;
			int warn = MessageBoxA(NULL, "This command will intentionally crash the program and may causes buffer overflow/stack overflow. Use for debugging or reverse engineering only.", "Warning", MB_OK | MB_ICONWARNING);
			if (warn == IDOK) {
                cout << "Crash Types:\n";
                cout << "1. Null Pointer Dereference\n";
                cout << "2. Buffer Overflow\n";
                cout << "3. Stack Overflow\n";
                cout << "4. Invalid Free\n";
				cout << "5. Divide by zero" << ANSI_RED << " DEPRECATED" << ANSI_RESET << "\n";
				cout << "6. Double Free\n";
				cout << "7. Access violation (Illegal address)\n";
				cout << "8. Unaligned memory access\n";
                cout << "9. Sending abort signal to Kernel ( abort(); ) \n";
                cout << "10. Segmentation Fault (Manually Raising Segmentation Fault)\n";
			    cout << "11. Cancel\n";
                cout << ANSI_BLUE << "0. More Information" << ANSI_RESET << "\n";
                cin >> crashChoice;
            }
			if (crashChoice == 1) {
				int* p = nullptr; *p = 42;
			}
			else if (crashChoice == 2) {
				char buffer[10]; for (int i = 0; i < 20; ++i) buffer[i] = 'A';
			}
			else if (crashChoice == 3) {
				kaboom();
			}
			else if (crashChoice == 4) {
                int* p = (int*)0x1234;
                delete p;
			}

			else if (crashChoice == 5) {
				MessageBoxA(NULL, "The 'Divide by zero' crash type is deprecated and will not be executed to prevent potential issues. Please choose another crash type.", "Deprecated", MB_OKCANCEL| MB_ICONINFORMATION);
				// int x = 1 / 0;   MSVC Got mad at us :DD
			}

            else if (crashChoice == 6) {
            
                int* p = (int*)malloc(sizeof(int));
                free(p); 
                free(p);
            }
            else if (crashChoice == 7) {

                int* p = (int*)0xFFFFFFFF;
                *p = 1;
            }

			else if (crashChoice == 8) {
                char* p = (char*)malloc(5);
                int* q = (int*)(p + 1); 
                *q = 123; 

			}
			else if (crashChoice == 9) {
				abort();
			}

            else if (crashChoice == 10)
            {
                raise(SIGSEGV);
            }

            else if (crashChoice == 11) {
				cout << "Crash canceled.\n";
            }

            else if (crashChoice == 0) {
                cout << "Crash Command Information:\n";
                cout << "1. Null Pointer Dereference: Attempts to write to a null pointer, causing an access violation. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "2. Buffer Overflow: Writing beyond the bounds of a buffer. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "3. Stack Overflow: Exhausting the call stack with too many nested function calls. Return Codes : "
                    << ANSI_YELLOW << "0xC00000FD alias -1073741571" << ANSI_RESET << "\n";

                cout << "4. Invalid Free: Attempting to free memory that was not allocated or already freed. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "5. " << ANSI_YELLOW << "Divide by zero: Attempting to divide an integer by zero (deprecated). Return Codes : "
                    << ANSI_YELLOW << "0xC0000094 alias -1073741676" << ANSI_RESET << "\n";

                cout << "6. Double Free: Attempting to free the same memory twice. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "7. Access violation: Attempting to access an illegal memory address. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "8. Unaligned memory access: Accessing memory with incorrect alignment. Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";

                cout << "9. Sending abort signal to Kernel: Using abort() to terminate the process and generate a crash dump. Return Codes : "
                    << ANSI_YELLOW << "0xC0000409 alias -1073740791" << ANSI_RESET << "\n";

                cout << "10. Segmentation Fault: Manually raising a segmentation fault using raise(SIGSEGV). Return Codes : "
                    << ANSI_YELLOW << "0xC0000005 alias -1073741819" << ANSI_RESET << "\n";
            }
            else {
				cout << "Crash command canceled.\n";
            }
        }

  else if (cmd == "format") {
      int formatChoice;
      std::string input;

      std::cout << "Number Format Conversions:\n";
      std::cout << "1. Decimal to Hexadecimal (32-bit)\n";
      std::cout << "2. Decimal to Hexadecimal (64-bit)\n";
      std::cout << "3. Hexadecimal to Decimal (32-bit)\n";
      std::cout << "4. Hexadecimal to Decimal (64-bit)\n";
      std::cout << "5. Decimal to Binary (32-bit)\n";
      std::cout << "6. Decimal to Binary (64-bit)\n";
      std::cout << "7. Binary to Decimal (32-bit)\n";
      std::cout << "8. Binary to Decimal (64-bit)\n";
      std::cout << "9. Binary to Hexadecimal (32-bit)\n";
      std::cout << "10. Binary to Hexadecimal (64-bit)\n";
      std::cout << "11. Hexadecimal to Binary (32-bit)\n";
      std::cout << "12. Hexadecimal to Binary (64-bit)\n";
      std::cout << "Enter your choice: ";
      std::cin >> formatChoice;

      std::cout << "Enter the number: ";
      std::cin >> input;

      switch (formatChoice) {
      case 1: {
          int val = std::stoi(input);
          std::cout << "Hex (trimmed): " << int2hex32_trim(val) << "\n";
          std::cout << "Hex (leading zeros): " << int2hex32(val) << "\n";
          break;
      }
      case 2: {
          long long val = std::stoll(input);
          std::cout << "Hex (trimmed): " << int2hex64_trim(val) << "\n";
          std::cout << "Hex (leading zeros): " << int2hex64(val) << "\n";
          break;
      }
      case 3: {
          int val = hex2int32(input);
          std::cout << "Decimal: " << val << "\n";
          break;
      }
      case 4: {
          long long val = hex2int64(input);
          std::cout << "Decimal: " << val << "\n";
          break;
      }
      case 5: {
          int val = std::stoi(input);
          std::cout << "Binary (trimmed): " << int2bin32_trim(val) << "\n";
          std::cout << "Binary (leading zeros): " << int2bin32(val) << "\n";
          break;
      }
      case 6: {
          long long val = std::stoll(input);
          std::cout << "Binary (trimmed): " << int2bin64_trim(val) << "\n";
          std::cout << "Binary (leading zeros): " << int2bin64(val) << "\n";
          break;
      }
      case 7: {
          int val = bin2int32(input);
          std::cout << "Decimal: " << val << "\n";
          break;
      }
      case 8: {
          long long val = bin2int64(input);
          std::cout << "Decimal: " << val << "\n";
          break;
      }
      case 9: {
          std::cout << "Hex (trimmed): " << bin2hex32(input) << "\n";
          std::cout << "Hex (leading zeros): " << int2hex32(bin2int32(input)) << "\n";
          break;
      }
      case 10: {
          std::cout << "Hex (trimmed): " << bin2hex64(input) << "\n";
          std::cout << "Hex (leading zeros): " << int2hex64(bin2int64(input)) << "\n";
          break;
      }
      case 11: {
          std::cout << "Binary (trimmed): " << hex2bin32(input).erase(0, hex2bin32(input).find_first_not_of('0')) << "\n";
          std::cout << "Binary (leading zeros): " << hex2bin32(input) << "\n";
          break;
      }
      case 12: {
          std::cout << "Binary (trimmed): " << hex2bin64(input).erase(0, hex2bin64(input).find_first_not_of('0')) << "\n";
          std::cout << "Binary (leading zeros): " << hex2bin64(input) << "\n";
          break;
      }
      default:
          std::cout << "Invalid choice!\n";
      }
}
        else {
            cout << "Unknown command: " << cmd << " (type help)\n";
        }
    }
    cout << "Goodbye.\n";    return 0;
}
