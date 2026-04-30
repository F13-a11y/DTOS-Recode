#include "scriptparser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

static void trim_inplace(std::string &s) {
    size_t b = 0; while (b < s.size() && isspace((unsigned char)s[b])) ++b;
    size_t e = s.size(); while (e > b && isspace((unsigned char)s[e-1])) --e;
    s = s.substr(b, e-b);
}

bool parse_dtos_file(const std::string &path, std::vector<DtosJob> &outJobs, std::string &err) {
    std::ifstream ifs(path);
    if (!ifs) { err = "failed to open file"; return false; }
    std::string line;
    while (std::getline(ifs, line)) {
        // remove BOM
        if (!line.empty() && (unsigned char)line[0] == 0xEF) {
            if (line.size() >= 3 && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) line = line.substr(3);
        }
        trim_inplace(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == ';') continue;
        std::string lower = line;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        const std::string prefix = "run ";
        if (lower.rfind(prefix, 0) != 0) continue; // unsupported line -> skip
        std::string payload = line.substr(prefix.size());
        std::string cmdpart = payload;
        int loops = 1;
        size_t comma = payload.find(',');
        if (comma != std::string::npos) {
            cmdpart = payload.substr(0, comma);
            std::string opt = payload.substr(comma + 1);
            trim_inplace(cmdpart); trim_inplace(opt);
            std::string optlow = opt; std::transform(optlow.begin(), optlow.end(), optlow.begin(), ::tolower);
            if (optlow.rfind("loop", 0) == 0) {
                std::string num = opt.substr(4);
                trim_inplace(num);
                try { loops = std::stoi(num); } catch(...) { loops = 1; }
                if (loops < 1) loops = 1;
            }
        } else {
            trim_inplace(cmdpart);
        }
        outJobs.push_back({cmdpart, loops});
    }
    return true;
}
