#pragma once

#include <sys/stat.h>

#include <string>
#include <map>

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../http_status.hpp"

namespace http {

typedef std::map<std::string, std::string, CaseInsensitiveLess> ExtensionMap;

struct FileInfo {
    std::string name;
    std::string path;
    std::string time;
    bool isDir;
    size_t size;
};

bool isDirectory(const struct stat& st);
bool isRegularFile(const struct stat& st);
std::string joinPath(const std::string& base, const std::string& path);
std::string getContentType(const std::string& filename,
                           const ExtensionMap& extensionMap);
void initExtensionMap(ExtensionMap& extensionMap);
HttpStatus::EHttpStatus checkFileAccess(const std::string& path,
                                        struct stat& st);
std::string getModifiedTime(const struct stat& st);
}  // namespace http
