#include <unistd.h>

#include <string>

#include "method_utils.hpp"

namespace http {

std::string joinPath(const std::string& base, const std::string& path) {
    if (base.empty()) {
        return path;
    }
    if (path.empty()) {
        return base;
    }
    if (base[base.size() - 1] == '/' || path[0] == '/') {
        return base + path;
    }
    return base + "/" + path;
}

HttpStatus::EHttpStatus checkFileAccess(const std::string& path,
                                        struct stat& st) {
    if (access(path.c_str(), F_OK) != 0) {
        return HttpStatus::NOT_FOUND;
    }
    if (stat(path.c_str(), &st) != 0) {
        return HttpStatus::INTERNAL_SERVER_ERROR;
    }
    if (!(st.st_mode & S_IRUSR) || access(path.c_str(), R_OK) != 0) {
        return HttpStatus::FORBIDDEN;
    }
    return HttpStatus::OK;
}

std::string findFirstExistingIndex(const std::string& path, const std::vector<std::string>& indices) {
    for (std::vector<std::string>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
        std::string fullPath = joinPath(path, *it);
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            return fullPath;
        }
    }
    return "";
}

std::string getModifiedTime(const struct stat& st) {
    char timeStr[26];
    ctime_r(&st.st_mtime, timeStr);
    timeStr[24] = '\0';  // del newline
    return std::string(timeStr);
}

}  // namespace http
