#include <string>
#include <iostream>

#include "../request/http_fields.hpp"
#include "server_method_handler.hpp"
// #include "head_method.hpp"
#include "method_utils.hpp"

namespace http {
namespace {
void handleDirectory(const std::string& path, std::vector<std::string> indices,
                    bool isAutoindex, Response& response) {
    HttpStatus::EHttpStatus status;

    if (!indices.empty()) {
        struct stat indexSt;
        std::string fullPath = findFirstExistingIndex(path, indices);
        status = checkFileAccess(fullPath, indexSt);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runHead: handleDirectory: checkFileAccess fail " + fullPath + " " + toolbox::to_string(status));
            throw status;
        }
        response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getInstance().getContentType(fullPath));
        response.setHeader(fields::LAST_MODIFIED, getModifiedTime(indexSt));
    }  else if (isAutoindex) {
        response.setHeader(fields::CONTENT_TYPE, "text/html");
    }
}

void handleFile(const std::string& path, Response& response) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        toolbox::logger::StepMark::error("runHead: handleFile: checkFileAccess fail "
            + path + " " + toolbox::to_string(status));
        throw status;
    }
    response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getInstance().getContentType(path));
    response.setHeader(fields::LAST_MODIFIED, getModifiedTime(st));
}
}  // namespace

namespace serverMethod {
void runHead(const std::string& path,
                                  std::vector<std::string> indices,
                                  bool isAutoindex,
                                  Response& response) {
    struct stat st;

    try {
        HttpStatus::EHttpStatus status = checkFileAccess(path, st);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runHead: checkFileAccess fail " + path + " " + toolbox::to_string(status));
            throw status;
        }

        if (isDirectory(st)) {
            handleDirectory(path, indices, isAutoindex, response);
        } else if (isRegularFile(st)) {
            handleFile(path, response);
        } else {
            throw HttpStatus::INTERNAL_SERVER_ERROR;
        }
        response.setStatus(HttpStatus::OK);
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runHead: set status "
            + toolbox::to_string(e));
        response.setStatus(e);
    }
}

}  // namespace serverMethod
}  // namespace http
