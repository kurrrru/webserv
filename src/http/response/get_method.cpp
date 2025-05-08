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

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"

struct FileInfo {
    std::string name;
    std::string path;
    time_t time;
    bool isDir;
    size_t size;
};

typedef std::map<std::string, std::string, http::CaseInsensitiveLess>
ExtentionMap;

bool isDirectory(const struct stat& st) {
    return S_ISDIR(st.st_mode);
}

bool isRegularFile(const struct stat& st) {
    return S_ISREG(st.st_mode);
}

void appendBody(FileInfo& info, std::string& responseBody) {
    std::string size = info.isDir ? "-" : std::to_string(info.size);
    responseBody += "<tr>\n"
                   "  <td><a href=\"" + info.path + "\">" + info.name + "</a></td>\n"
                   "  <td>" + size + "</td>\n"
                   "  <td>" + ctime(&info.time) + "</td>\n"
                   "</tr>\n";
}

std::string readFile(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            throw std::runtime_error("File not found: " + path);
        } else if (errno == EACCES) {
            throw std::runtime_error("Permission denied: " + path);
        }
    }
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getContentType(const std::string& filename,
    ExtentionMap& extentionMap) {
    size_t pos = filename.find_last_of(".");
    if (pos == std::string::npos || pos == filename.size()) {
        return "application/octet-stream";
    }
    std::string ext = filename.substr(pos + 1);
    ExtentionMap::iterator it = extentionMap.find(ext);
    if (it != extentionMap.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

void readDirectoryEntries(const std::string& dirPath,
    std::string& responseBody) {
    struct stat st;
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        throw std::runtime_error("Could not open directory: " +
            dirPath + " - " + strerror(errno));
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        FileInfo info;
        info.name = entry->d_name;
        if (dirPath[dirPath.size() - 1] != '/') {
            info.path = dirPath + "/" + info.name;
        } else {
            info.path = dirPath + info.name;
        }
        if (stat(info.path.c_str(), &st) != 0) {
            closedir(dir);
            throw std::runtime_error("Error accessing file: " +
                info.path + " - " + strerror(errno));
        }
        info.time = st.st_mtime;
        info.isDir = S_ISDIR(st.st_mode);
        info.size = st.st_size;
        appendBody(info, responseBody);
    }
    closedir(dir);
}

std::string processAutoindex(const std::string& path) {
    std::string responseBody;
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
    return responseBody;
}

http::HttpStatus executeGet(const std::string& path, std::string& responseBody,
    const std::string& indexPath, bool isAutoindex, std::string& contentType,
    ExtentionMap& extentionMap) {
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

    if (isDirectory(st)) {
        if (!indexPath.empty()) {
            struct stat indexSt;
            if (stat((path + indexPath).c_str(), &indexSt) != 0) {
                return http::NOT_FOUND;
            }
            if (!(indexSt.st_mode & S_IRUSR)) {
                return http::FORBIDDEN;
            }
            responseBody = readFile(path + indexPath);
            contentType = getContentType(indexPath, extentionMap);
        }  else if (isAutoindex) {
            try {
                responseBody = processAutoindex(path);
                contentType = "text/html";
            } catch (const std::exception& e) {
                return http::INTERNAL_SERVER_ERROR;
            }
        }
    } else {
        responseBody = readFile(path);
        contentType = getContentType(indexPath, extentionMap);
    }
    return http::OK;
}

void initExtentionMap(ExtentionMap& extentionMap) {
    extentionMap["html"] = "text/html";
    extentionMap["htm"] = "text/html";
    extentionMap["css"] = "text/css";
    extentionMap["js"] = "application/javascript";
    extentionMap["json"] = "application/json";
    extentionMap["txt"] = "text/plain";
    extentionMap["jpg"] = "image/jpeg";
    extentionMap["jpeg"] = "image/jpeg";
    extentionMap["png"] = "image/png";
    extentionMap["gif"] = "image/gif";
    extentionMap["svg"] = "image/svg+xml";
    extentionMap["ico"] = "image/x-icon";
    extentionMap["pdf"] = "application/pdf";
    extentionMap["xml"] = "application/xml";
    extentionMap["zip"] = "application/zip";
    extentionMap["gz"] = "application/gzip";
    extentionMap["tar"] = "application/x-tar";
    extentionMap["mp3"] = "audio/mpeg";
    extentionMap["mp4"] = "video/mp4";
    extentionMap["avi"] = "video/x-msvideo";
    extentionMap["php"] = "application/x-httpd-php";
    extentionMap["py"] = "text/x-python";
    extentionMap["c"] = "text/x-c";
    extentionMap["cpp"] = "text/x-c++";
    extentionMap["h"] = "text/x-c";
    extentionMap["hpp"] = "text/x-c++";
}

int main(void) {
    std::string path;
    std::string responseBody;
    std::string indexPath;
    std::string contentType;
    ExtentionMap extentionMap;

    path = "/home/yooshima/42cursus/webserv/docs/html";
    // path = "/home/yooshima/42cursus/webserv/docs/html/index.html";
    // indexPath = "index.html";
    indexPath = "";
    initExtentionMap(extentionMap);
    http::HttpStatus status = executeGet(path, responseBody, indexPath, true, contentType, extentionMap);
    std::cout << "content-type = " << contentType << std::endl;
    std::cout << "status = " << status << std::endl;
    std::cout << responseBody;
}