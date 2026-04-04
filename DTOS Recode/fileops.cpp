#include "fileops.h"
#include "ofn.h"
#include <filesystem>
#include <string>

int delete_via_ofn(void) {
    // Use ofn to let the user pick a file. Do not include system filters.
    std::string path = ofn(false);
    if (path.empty()) return 0;
    try {
        std::filesystem::path p = std::filesystem::path(path);
        if (std::filesystem::exists(p) && !std::filesystem::is_directory(p)) {
            std::error_code ec;
            bool ok = std::filesystem::remove(p, ec);
            if (ok && !ec) return 1;
            return 0;
        }
    } catch (...) {
    }
    return 0;
}
