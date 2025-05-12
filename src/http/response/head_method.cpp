#include <string>

#include "../request/http_fields.hpp"
#include <method_utils.hpp>

namespace http {
HttpStatus::EHttpStatus handleDirectory(const std::string& path,
                                        const std::string& indexPath,
                                        bool isAutoindex,
                                        ExtensionMap& extensionMap,
                                        HTTPFields& responseFields) {
    HttpStatus::EHttpStatus status;
    if (!indexPath.empty()) {
        struct stat indexSt;
        status = checkFileAccess(path + indexPath, indexSt);
        if (status != HttpStatus::OK) {
            return status;
        }
        responseFields.getFieldValue("Content-Length").push_back(
            toolbox::to_string(indexSt.st_size));
        responseFields.getFieldValue("Content-Type").push_back(
            getContentType(indexPath, extensionMap));
    }  else if (isAutoindex) {
        responseFields.getFieldValue("Content-Type").push_back("text/html");
    }
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus handleFile(const std::string& path,
                                    ExtensionMap& extensionMap,
                                    HTTPFields& responseFields) {
    struct stat st;
    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        return status;
    }
    responseFields.getFieldValue("Content-Length").push_back(
        toolbox::to_string(st.st_size));
    responseFields.getFieldValue("Content-Type").push_back(
        getContentType(path, extensionMap));
    return HttpStatus::OK;
}

HttpStatus::EHttpStatus runHead(const std::string& path,
                               std::string& responseBody,
                               const std::string& indexPath,
                               bool isAutoindex,
                               ExtensionMap& extensionMap,
                               HTTPFields& responseFields) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        return status;
    }

    try {
        if (isDirectory(st)) {
            status = handleDirectory(path, indexPath, isAutoindex,
                extensionMap, responseFields);
        } else if (isRegularFile(st)) {
            status = handleFile(path, extensionMap, responseFields);
        } else {
            status = HttpStatus::INTERNAL_SERVER_ERROR;
        }
    } catch (std::exception& e) {
        status = HttpStatus::INTERNAL_SERVER_ERROR;
    }
    return status;
}

}  // namespace http
