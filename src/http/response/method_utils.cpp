#include <string>
#include <vector>
#include <ctime>

#include <unistd.h>

#include "method_utils.hpp"

namespace http {

std::string joinPath(const std::string& base, const std::string& path) {
    if (base.empty()) {
        return path;
    }
    if (path.empty()) {
        return base;
    }
    if (path[0] == '/') {
        if (path.length() == 1) {
            if (base[base.size() - 1] == '/') {
                return base.substr(0, base.size() - 1) + path;
            } else {
                return base + path;
            }
        } else {
            std::string pathWithoutSlash = path.substr(1);
            if (base[base.size() - 1] == '/') {
                return base + pathWithoutSlash;
            } else {
                return base + "/" + pathWithoutSlash;
            }
        }
    }

    if (base[base.size() - 1] == '/' || path[0] == '/') {
        return base + path;
    }
    return base + "/" + path;
}

HttpStatus::EHttpStatus checkFileAccess(const std::string& path,
                                        struct stat& st) {
    if (path.empty() || access(path.c_str(), F_OK) != 0) {
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

std::string findFirstExistingIndex(const std::string& path,
    const std::vector<std::string>& indices) {
    for (std::size_t i = 0; i != indices.size(); ++i) {
        std::string fullPath = joinPath(path, indices[i]);
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            return fullPath;
        }
    }
    return "";
}

std::string getModifiedTime(const struct stat& st) {
    char buffer[32];
    std::tm* fileTime = std::localtime(&st.st_mtime);

    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", fileTime);
    return std::string(buffer);
}

}  // namespace http
