#include <string>
#include <iostream>

#include "cgi_response.hpp"

namespace http {
static bool isLocalURI(const std::string& path) {
    return (path.size() >= 2 && path[0] == '/' && path[1] != '/');
}

static bool isAbsoluteURI(const std::string& path) {
    return path.find("://") != std::string::npos;
}

static bool isClientRedirectStatus(http::HttpStatus::EHttpStatus status) {
    return (status == HttpStatus::MOVED_PERMANENTLY ||
            status == HttpStatus::FOUND ||
            status == HttpStatus::SEE_OTHER ||
            status == HttpStatus::TEMPORARY_REDIRECT ||
            status == HttpStatus::PERMANENT_REDIRECT);
}

void CgiResponse::identifyCgiType() {
    if (httpStatus.get() == HttpStatus::INTERNAL_SERVER_ERROR) {
        return;
    }
    HTTPFields::FieldValue location = fields.getFieldValue(fields::LOCATION);
    bool emptyContentType = fields.getFieldValue(fields::CONTENT_TYPE).empty();
    bool isStatusUnset = httpStatus.get() == HttpStatus::UNSET;
    if (location.empty()) {
        if (!emptyContentType) {
            cgiType = DOCUMENT;
            if (isStatusUnset) {
                httpStatus.set(HttpStatus::OK);
            }
        }
        return;
    }
    bool isOnlyLocation = fields.countNonEmptyValues() == 1;
    bool hasEmptyBody = body.empty();
    if (isLocalURI(location[0]) && isOnlyLocation &&
        hasEmptyBody && isStatusUnset) {
        cgiType = LOCAL_REDIRECT;
        return;
    }
    if (!isAbsoluteURI(location[0])) {
        return;
    }
    if (isOnlyLocation && isStatusUnset) {
        cgiType = CLIENT_REDIRECT;
        return;
    }
    if (!emptyContentType && !hasEmptyBody &&
        isClientRedirectStatus(httpStatus.get())) {
        cgiType = CLIENT_REDIRECT_DOCUMENT;
    }
}

}  // namespace http
