#include <unistd.h>

#include <string>

#include "method_utils.hpp"

namespace http {
bool isDirectory(const struct stat& st) {
    return S_ISDIR(st.st_mode);
}

bool isRegularFile(const struct stat& st) {
    return S_ISREG(st.st_mode);
}

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

std::string getContentType(const std::string& filename,
                           const ExtensionMap& extensionMap) {
    size_t pos = filename.find_last_of(".");
    if (pos == std::string::npos || pos == filename.size()) {
        return "application/octet-stream";
    }
    std::string ext = filename.substr(pos + 1);
    ExtensionMap::const_iterator it = extensionMap.find(ext);
    if (it != extensionMap.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

void initExtensionMap(ExtensionMap& extensionMap) {
    extensionMap["html"] = "text/html";
    extensionMap["htm"] = "text/html";
    extensionMap["css"] = "text/css";
    extensionMap["js"] = "application/javascript";
    extensionMap["json"] = "application/json";
    extensionMap["txt"] = "text/plain";
    extensionMap["jpg"] = "image/jpeg";
    extensionMap["jpeg"] = "image/jpeg";
    extensionMap["png"] = "image/png";
    extensionMap["gif"] = "image/gif";
    extensionMap["svg"] = "image/svg+xml";
    extensionMap["ico"] = "image/x-icon";
    extensionMap["pdf"] = "application/pdf";
    extensionMap["xml"] = "application/xml";
    extensionMap["zip"] = "application/zip";
    extensionMap["gz"] = "application/gzip";
    extensionMap["tar"] = "application/x-tar";
    extensionMap["mp3"] = "audio/mpeg";
    extensionMap["mp4"] = "video/mp4";
    extensionMap["avi"] = "video/x-msvideo";
    extensionMap["php"] = "application/x-httpd-php";
    extensionMap["py"] = "text/x-python";
    extensionMap["c"] = "text/x-c";
    extensionMap["cpp"] = "text/x-c++";
    extensionMap["h"] = "text/x-c";
    extensionMap["hpp"] = "text/x-c++";
}

HttpStatus::EHttpStatus checkFileAccess(const std::string& path,
                                        struct stat& st) {
    if (access(path.c_str(), F_OK) != 0) {
        return HttpStatus::NOT_FOUND;
    }
    if (stat(path.c_str(), &st) != 0) {
        throw std::runtime_error("stat() failed");
    }
    if (!(st.st_mode & S_IRUSR) || access(path.c_str(), R_OK) != 0) {
        return HttpStatus::FORBIDDEN;
    }
    return HttpStatus::OK;
}

std::string getModifiedTime(const struct stat& st) {
    char timeStr[26];
    ctime_r(&st.st_mtime, timeStr);
    timeStr[24] = '\0';  // del newline
    return std::string(timeStr);
}

}  // namespace http
