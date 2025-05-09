#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <cstring>

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"

typedef std::map<std::string, std::string, http::CaseInsensitiveLess>
ExtensionMap;

struct FileInfo {
    std::string name;
    std::string path;
    time_t time;
    bool isDir;
    size_t size;
};

void readDirectoryEntries(const std::string& dirPath,
    std::string& responseBody);

// Utility functions

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

std::string getCurrentWorkingDir() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != NULL) {
        return std::string(buffer);
    }
    return ".";
}

// Extension map functions

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

// File handling functions

void readFile(const std::string& path, std::string& responseBody) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        toolbox::logger::StepMark::error("GetMethod: cannot readFile");
        throw std::runtime_error("");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    responseBody += buffer.str();
}

// Directory handling functions

void appendFileInfoRow(FileInfo& info, std::string& responseBody) {
    std::string size = info.isDir ? "-" : toolbox::to_string(info.size);
    char timeStr[26];
    ctime_r(&info.time, timeStr);
    timeStr[24] = '\0';  // del newline
    std::stringstream ss;
    ss << "<tr>\n"
        << "  <td><a href=\"" << info.path << "\">"
        << info.name << "</a></td>\n"
        << "  <td>" << size << "</td>\n"
        << "  <td>" << timeStr << "</td>\n"
        << "</tr>\n";
    responseBody += ss.str();
}

void processAutoindex(const std::string& path, std::string& responseBody) {
    responseBody += "<!DOCTYPE html>\n"
                   "<html>\n"
                   "<head>\n"
                   "  <title>Index of " + path + "</title>\n"
                   "  <style>\n"
                   "    body { font-family: Arial, sans-serif; margin: 20px; }\n"
                   "    h1 { color: #333; }\n"
                   "    table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n"
                   "    th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }\n"
                   "    th { background-color: #f5f5f5; }\n"
                   "    a { color: #0066cc; text-decoration: none; }\n"
                   "    a:hover { text-decoration: underline; }\n"
                   "  </style>\n"
                   "</head>\n"
                   "<body>\n"
                   "  <h1>Index of " + path + "</h1>\n"
                   "  <table>\n"
                   "    <tr>\n"
                   "      <th>Name</th>\n"
                   "      <th>Size</th>\n"
                   "      <th>Last Modified</th>\n"
                   "    </tr>\n";
    readDirectoryEntries(path, responseBody);
    responseBody += "  </table>\n"
                   "</body>\n"
                   "</html>\n";
}

void readDirectoryEntries(const std::string& dirPath,
    std::string& responseBody) {
    struct stat st;
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        toolbox::logger::StepMark::error("GetMethod: failed to open directory");
        throw std::runtime_error("");
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        FileInfo info;
        info.name = entry->d_name;
        info.path = joinPath(dirPath, info.name);
        if (stat(info.path.c_str(), &st) != 0) {
            closedir(dir);
            toolbox::logger::StepMark::error("GetMethod: failed to access");
            throw std::runtime_error("");
        }
        info.time = st.st_mtime;
        info.isDir = S_ISDIR(st.st_mode);
        info.size = st.st_size;
        appendFileInfoRow(info, responseBody);
    }
    closedir(dir);
}

// Main processing functions

http::HttpStatus checkFileAccess(const std::string& path, struct stat& st) {
    if (stat(path.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            return http::NOT_FOUND;
        } else if (errno == EACCES) {
            return http::FORBIDDEN;
        }
        return http::INTERNAL_SERVER_ERROR;
    }
    if (!(st.st_mode & S_IRUSR)) {
        return http::FORBIDDEN;
    }
    return http::OK;
}

http::HttpStatus handleDirectory(const std::string& path,
    const std::string& indexPath, std::string& responseBody,
    std::string& contentType, ExtensionMap& extensionMap, bool isAutoindex) {
    http::HttpStatus status;
    if (!indexPath.empty()) {
        struct stat indexSt;
        status = checkFileAccess(path + indexPath, indexSt);
        if (status != http::OK) {
            return status;
        }
        readFile(path + indexPath, responseBody);
        contentType = getContentType(indexPath, extensionMap);
    }  else if (isAutoindex) {
        processAutoindex(path, responseBody);
        contentType = "text/html";
    }
    return http::OK;
}

http::HttpStatus handleFile(const std::string& path, std::string& responseBody,
    std::string& contentType, ExtensionMap& extensionMap) {
    readFile(path, responseBody);
    contentType = getContentType(path, extensionMap);
    return http::OK;
}

http::HttpStatus runGet(const std::string& path, std::string& responseBody,
    const std::string& indexPath, bool isAutoindex, std::string& contentType,
    ExtensionMap& extensionMap) {
    struct stat st;

    http::HttpStatus status = checkFileAccess(path, st);
    if (status != http::OK) {
        return status;
    }
    try {
        if (isDirectory(st)) {
            return handleDirectory(path, indexPath, responseBody, contentType,
                extensionMap, isAutoindex);
        } else if (isRegularFile(st)) {
            return handleFile(path, responseBody, contentType, extensionMap);
        } else {
            status = http::INTERNAL_SERVER_ERROR;
        }
    } catch (const std::exception& e) {
        status = http::INTERNAL_SERVER_ERROR;
    }
    return status;
}
