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

#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"
#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "method_utils.hpp"
#include "get_method.hpp"

namespace http {
std::string readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("file open error " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Directory handling functions

void buildFileInfoRow(FileInfo& info, std::stringstream& ss) {
    std::string size = info.isDir ? "-" : toolbox::to_string(info.size);
    ss << "<tr>\n"
       << "  <td><a href=\"" << info.path << "\">"
       << info.name << "</a></td>\n"
       << "  <td>" << size << "</td>\n"
       << "  <td>" << info.time << "</td>\n"
       << "</tr>\n";
}

void readDirectoryEntries(const std::string& dirPath, std::stringstream& ss) {
    struct stat st;
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        throw std::runtime_error("failed to open directory " + dirPath);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        FileInfo info;
        info.name = entry->d_name;
        info.path = joinPath(dirPath, info.name);
        if (stat(info.path.c_str(), &st) != 0) {
            throw std::runtime_error("failed to access " + info.path);
        }
        info.time = getModifiedTime(st);
        info.isDir = S_ISDIR(st.st_mode);
        info.size = st.st_size;
        buildFileInfoRow(info, ss);
    }
    closedir(dir);
}

std::string processAutoindex(const std::string& path) {
    std::stringstream ss;
    ss << "<!DOCTYPE html>\n"
       << "<html>\n"
       << "<head>\n"
       << "  <title>Index of " << path << "</title>\n"
       << "  <style>\n"
       << "    body { font-family: Arial, sans-serif; margin: 20px; }\n"
       << "    h1 { color: #333; }\n"
       << "    table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n"
       << "    th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }\n"
       << "    th { background-color: #f5f5f5; }\n"
       << "    a { color: #0066cc; text-decoration: none; }\n"
       << "    a:hover { text-decoration: underline; }\n"
       << "  </style>\n"
       << "</head>\n"
       << "<body>\n"
       << "  <h1>Index of " << path << "</h1>\n"
       << "  <table>\n"
       << "    <tr>\n"
       << "      <th>Name</th>\n"
       << "      <th>Size</th>\n"
       << "      <th>Last Modified</th>\n"
       << "    </tr>\n";

    readDirectoryEntries(path, ss);

    ss << "  </table>\n"
       << "</body>\n"
       << "</html>\n";
    return ss.str();
}

HttpStatus::EHttpStatus handleDirectory(const std::string& path,
                                        const std::string& indexPath,
                                        Response& response,
                                        ExtensionMap& extensionMap,
                                        bool isAutoindex) {
    HttpStatus::EHttpStatus status;

    if (!indexPath.empty()) {
        struct stat indexSt;
        std::string fullPath = joinPath(path, indexPath);
        status = checkFileAccess(fullPath, indexSt);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runGet: checkFileAccess fail "
                + fullPath + " " + toolbox::to_string(status));
            return status;
        }
        response.setBody(readFile(fullPath));
        response.setHeader(fields::CONTENT_TYPE, getContentType(fullPath, extensionMap));
        response.setHeader(fields::LAST_MODIFIED, getModifiedTime(indexSt));
    }  else if (isAutoindex) {
        response.setBody(processAutoindex(path));
        response.setHeader(fields::CONTENT_TYPE, "text/html");
    }
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus handleFile(const std::string& path,
                                   Response& response,
                                   ExtensionMap& extensionMap) {
    response.setBody(readFile(path));
    response.setHeader(fields::CONTENT_TYPE,getContentType(path, extensionMap));
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus runGet(const std::string& path,
                               const std::string& indexPath,
                               bool isAutoindex,
                               ExtensionMap& extensionMap,
                               Response& response) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        toolbox::logger::StepMark::error("runGet: checkFileAccess fail " + path
            + " " + toolbox::to_string(status));
        return status;
    }

    try {
        if (isDirectory(st)) {
            return handleDirectory(path, indexPath, response,
                extensionMap, isAutoindex);
        } else if (isRegularFile(st)) {
            return handleFile(path, response, extensionMap);
        } else {
            status = HttpStatus::INTERNAL_SERVER_ERROR;
        }
    } catch (const std::exception& e) {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
        toolbox::logger::StepMark::error("runGet: exception "
            + std::string(e.what()) + " " + toolbox::to_string(status));
    }
    return status;
}

}  // namespace http
