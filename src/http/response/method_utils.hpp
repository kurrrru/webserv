// Copyright 2025 Ideal Broccoli

#pragma once

#include <sys/stat.h>

#include <string>
#include <vector>
#include <map>

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"

namespace http {

typedef std::map<std::string, std::string, CaseInsensitiveLess> ExtensionMap;

struct FileInfo {
    std::string name;
    std::string path;
    std::string time;
    bool isDir;
    std::size_t size;
};

inline bool isDirectory(const struct stat& st) { return S_ISDIR(st.st_mode); }
inline bool isRegularFile(const struct stat& st) { return S_ISREG(st.st_mode); }

std::string joinPath(const std::string& base, const std::string& path);
HttpStatus::EHttpStatus checkFileAccess(const std::string& path, struct stat& st);
std::string getModifiedTime(const struct stat& st);
std::string findFirstExistingIndex(const std::string& path, const std::vector<std::string>& indices);
}  // namespace http
