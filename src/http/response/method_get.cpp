#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <cstring>

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"
#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "method_utils.hpp"
#include "server_method_handler.hpp"

namespace http {
namespace {
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

void handleDirectory(const std::string& path, std::vector<std::string>& indices,
                     Response& response, bool isAutoindex) {
    HttpStatus::EHttpStatus status;

    if (!indices.empty()) {
        struct stat indexSt;
        std::string fullPath = findFirstExistingIndex(path, indices);
        status = checkFileAccess(fullPath, indexSt);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runGet: checkFileAccess fail "
                + fullPath + " " + toolbox::to_string(status));
            throw status;
        }
        response.setBody(readFile(fullPath));
        response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getInstance().getContentType(fullPath));
        response.setHeader(fields::LAST_MODIFIED, getModifiedTime(indexSt));
    }  else if (isAutoindex) {
        response.setBody(processAutoindex(path));
        response.setHeader(fields::CONTENT_TYPE, "text/html");
    }
}

void handleFile(const std::string& path, Response& response) {
    response.setBody(readFile(path));
    response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getInstance().getContentType(path));
}
}  // namespace

namespace serverMethod {
void runGet(const std::string& path, std::vector<std::string>& indices,
        bool isAutoindex, Response& response) {
    struct stat st;

    try {
        HttpStatus::EHttpStatus status = checkFileAccess(path, st);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runGet: checkFileAccess fail "
                + path + " " + toolbox::to_string(status));
            throw status;
        }

        if (isDirectory(st)) {
            handleDirectory(path, indices, response, isAutoindex);
        } else if (isRegularFile(st)) {
            handleFile(path, response);
        } else {
            throw HttpStatus::INTERNAL_SERVER_ERROR;
        }
        response.setStatus(HttpStatus::OK);
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runGet: set status "
            + toolbox::to_string(e));
        response.setStatus(e);
    }
}

}  // namespace serverMethod
}  // namespace http
