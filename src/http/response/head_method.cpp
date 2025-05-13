#include <string>
#include <iostream>

#include "../request/http_fields.hpp"
#include "head_method.hpp"
#include "method_utils.hpp"

namespace http {
HttpStatus::EHttpStatus handleDirectory(const std::string& path,
                                        const std::string& indexPath,
                                        bool isAutoindex,
                                        ExtensionMap& extensionMap,
                                        Response& response) {
    HttpStatus::EHttpStatus status;

    if (!indexPath.empty()) {
        struct stat indexSt;
        std::string fullPath = joinPath(path, indexPath);
        status = checkFileAccess(fullPath, indexSt);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runHead: checkFileAccess fail "
                + fullPath + " " + toolbox::to_string(status));
            return status;
        }
        response.setHeader(fields::CONTENT_TYPE, getContentType(fullPath, extensionMap));
        response.setHeader(fields::LAST_MODIFIED, getModifiedTime(indexSt));
    }  else if (isAutoindex) {
        response.setHeader(fields::CONTENT_TYPE, "text/html");
    }
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus handleFile(const std::string& path,
                                    ExtensionMap& extensionMap,
                                    Response& response) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        toolbox::logger::StepMark::error("runHead: checkFileAccess fail "
            + path + " " + toolbox::to_string(status));
        return status;
    }
    response.setHeader(fields::CONTENT_TYPE, getContentType(path, extensionMap));
    response.setHeader(fields::LAST_MODIFIED, getModifiedTime(st));
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus runHead(const std::string& path,
                               const std::string& indexPath,
                               bool isAutoindex,
                               ExtensionMap& extensionMap,
                               Response& response) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        toolbox::logger::StepMark::error("runHead: checkFileAccess fail "
            + path + " " + toolbox::to_string(status));
        return status;
    }

    try {
    if (isDirectory(st)) {
        status = handleDirectory(path, indexPath, isAutoindex,
            extensionMap, response);
    } else if (isRegularFile(st)) {
        status = handleFile(path, extensionMap, response);
    } else {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
    }
    } catch (const std::exception& e) {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
        toolbox::logger::StepMark::error("runHead: exception "
            + std::string(e.what()) + " " + toolbox::to_string(status));
    }
    return status;
}

}  // namespace http
