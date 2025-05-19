#include <string>
#include <iostream>

#include "../request/http_fields.hpp"
#include "head_method.hpp"
#include "method_utils.hpp"

namespace http {
namespace {
void handleDirectory(const std::string& path, const std::string& indexPath,
                    bool isAutoindex, Response& response) {
    HttpStatus::EHttpStatus status;

    if (!indexPath.empty()) {
        struct stat indexSt;
        std::string fullPath = joinPath(path, indexPath);
        status = checkFileAccess(fullPath, indexSt);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runHead: handleDirectory: checkFileAccess fail " + fullPath + " " + toolbox::to_string(status));
            throw status;
        }
        response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getContentType(fullPath));
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
    response.setHeader(fields::CONTENT_TYPE, ContentTypeManager::getContentType(path));
    response.setHeader(fields::LAST_MODIFIED, getModifiedTime(st));
}
}  // namespace

void runHead(const std::string& path, const std::string& indexPath,
            bool isAutoindex, Response& response) {
    struct stat st;

    try {
        HttpStatus::EHttpStatus status = checkFileAccess(path, st);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runHead: runHead: checkFileAccess fail " + path + " " + toolbox::to_string(status));
            throw status;
        }

        if (isDirectory(st)) {
            handleDirectory(path, indexPath, isAutoindex, response);
        } else if (isRegularFile(st)) {
            handleFile(path, response);
        } else {
            throw HttpStatus::INTERNAL_SERVER_ERROR;
        }
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runHead: set status "
            + toolbox::to_string(e));
        response.setStatus(e);
    }
    response.setStatus(HttpStatus::OK);
}

}  // namespace http
