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
#include "../http_namespace.hpp"
#include "method_utils.hpp"
#include "get_method.hpp"

namespace http {
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
    std::stringstream ss;
    ss << "<tr>\n"
        << "  <td><a href=\"" << info.path << "\">"
        << info.name << "</a></td>\n"
        << "  <td>" << size << "</td>\n"
        << "  <td>" << info.time << "</td>\n"
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
        info.time = getModifiedTime(st);
        info.isDir = S_ISDIR(st.st_mode);
        info.size = st.st_size;
        appendFileInfoRow(info, responseBody);
    }
    closedir(dir);
}

HttpStatus::EHttpStatus handleDirectory(const std::string& path,
                                        const std::string& indexPath,
                                        std::string& responseBody,
                                        std::string& contentType,
                                        ExtensionMap& extensionMap,
                                        bool isAutoindex) {
    HttpStatus::EHttpStatus status;
    if (!indexPath.empty()) {
        struct stat indexSt;
        status = checkFileAccess(path + indexPath, indexSt);
        if (status != HttpStatus::OK) {
            return status;
        }
        readFile(path + indexPath, responseBody);
        contentType = getContentType(indexPath, extensionMap);
    }  else if (isAutoindex) {
        processAutoindex(path, responseBody);
        contentType = "text/html";
    }
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus handleFile(const std::string& path,
                                   std::string& responseBody,
                                   std::string& contentType,
                                   ExtensionMap& extensionMap) {
    readFile(path, responseBody);
    contentType = getContentType(path, extensionMap);
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus runGet(const std::string& path,
                               std::string& responseBody,
                               const std::string& indexPath,
                               bool isAutoindex,
                               std::string& contentType,
                               ExtensionMap& extensionMap) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        return status;
    }

    try {
        if (isDirectory(st)) {
            return handleDirectory(path, indexPath, responseBody, contentType,
                extensionMap, isAutoindex);
        } else if (isRegularFile(st)) {
            return handleFile(path, responseBody, contentType, extensionMap);
        } else {
            status = HttpStatus::INTERNAL_SERVER_ERROR;
        }
    } catch (const std::exception& e) {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
    }
    return status;
}

}  // namespace http
