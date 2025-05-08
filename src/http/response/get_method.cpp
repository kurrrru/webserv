#include <sys/stat.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <dirent.h>
#include <ctime>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"
void readDirectoryEntries(const std::string& dirPath,
    std::string& responseBody);

struct FileInfo {
    std::string name;
    std::string path;
    time_t time;
    bool isDir;
    size_t size;
};

typedef std::map<std::string, std::string, http::CaseInsensitiveLess>
ExtentionMap;

// utils

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

std::string getContentType(const std::string& filename,
    const ExtentionMap& extensionMap) {
    size_t pos = filename.find_last_of(".");
    if (pos == std::string::npos || pos == filename.size()) {
        return "application/octet-stream";
    }
    std::string ext = filename.substr(pos + 1);
    ExtentionMap::const_iterator it = extensionMap.find(ext);
    if (it != extensionMap.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

//  extension map

void initExtensionMap(ExtentionMap& extensionMap) {
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

//  append body

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

void appendFileInfoRow(FileInfo& info, std::string& responseBody) {
    std::string size = info.isDir ? "-" : toolbox::to_string(info.size);
    char timeStr[26];
    ctime_r(&info.time, timeStr);
    timeStr[24] = '\0';  // del newline
    responseBody += "<tr>\n"
                   "  <td><a href=\"" + info.path + "\">" +
                   info.name + "</a></td>\n"
                   "  <td>" + size + "</td>\n"
                   "  <td>" + timeStr + "</td>\n"
                   "</tr>\n";
}

//  file handling

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

http::HttpStatus executeGet(const std::string& path, std::string& responseBody,
    const std::string& indexPath, bool isAutoindex, std::string& contentType,
    ExtentionMap& extensionMap) {

    struct stat st;
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
    try {
        if (isDirectory(st)) {
            if (!indexPath.empty()) {
                struct stat indexSt;
                if (stat((path + indexPath).c_str(), &indexSt) != 0) {
                    return http::NOT_FOUND;
                }
                if (!(indexSt.st_mode & S_IRUSR)) {
                    return http::FORBIDDEN;
                }
                readFile(path + indexPath, responseBody);
                contentType = getContentType(indexPath, extensionMap);
            }  else if (isAutoindex) {
                    processAutoindex(path, responseBody);
                    contentType = "text/html";
            }
        } else {
            readFile(path, responseBody);
            contentType = getContentType(path, extensionMap);
        }
    } catch (const std::exception& e) {
        return http::INTERNAL_SERVER_ERROR;
    }
    return http::OK;
}
