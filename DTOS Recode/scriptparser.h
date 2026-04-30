#pragma once
#include <string>
#include <vector>

struct DtosJob { std::string command; int loops; };

// Parse a .dtos file into jobs. On success returns true and fills outJobs.
// On failure returns false and fills err with a brief message.
bool parse_dtos_file(const std::string &path, std::vector<DtosJob> &outJobs, std::string &err);
