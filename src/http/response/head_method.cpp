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
        status = checkFileAccess(path + indexPath, indexSt);
        if (status != HttpStatus::OK) {
            return status;
        }
        response.setHeader(fields::CONTENT_TYPE, getContentType(indexPath, extensionMap));
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
    } catch (std::exception& e) {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
    }
    return status;
}

}  // namespace http
