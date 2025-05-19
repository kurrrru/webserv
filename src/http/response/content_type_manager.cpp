#include "content_type_manager.hpp"

namespace http {

std::map<std::string, std::string> ContentTypeManager::_contentTypeMap;

namespace {
std::string extractExtension(const std::string& filename) {
    std::size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "";
    }
    std::string extension = filename.substr(dot_pos + 1);
    return extension;
}
}

void ContentTypeManager::initContentTypeMap() {
    _contentTypeMap["html"] = "text/html";
    _contentTypeMap["htm"] = "text/html";
    _contentTypeMap["css"] = "text/css";
    _contentTypeMap["js"] = "application/javascript";
    _contentTypeMap["json"] = "application/json";
    _contentTypeMap["txt"] = "text/plain";
    _contentTypeMap["jpg"] = "image/jpeg";
    _contentTypeMap["jpeg"] = "image/jpeg";
    _contentTypeMap["png"] = "image/png";
    _contentTypeMap["gif"] = "image/gif";
    _contentTypeMap["svg"] = "image/svg+xml";
    _contentTypeMap["ico"] = "image/x-icon";
    _contentTypeMap["pdf"] = "application/pdf";
    _contentTypeMap["xml"] = "application/xml";
    _contentTypeMap["zip"] = "application/zip";
    _contentTypeMap["gz"] = "application/gzip";
    _contentTypeMap["tar"] = "application/x-tar";
    _contentTypeMap["mp3"] = "audio/mpeg";
    _contentTypeMap["mp4"] = "video/mp4";
    _contentTypeMap["avi"] = "video/x-msvideo";
    _contentTypeMap["php"] = "application/x-httpd-php";
    _contentTypeMap["py"] = "text/x-python";
    _contentTypeMap["c"] = "text/x-c";
    _contentTypeMap["cpp"] = "text/x-c++";
    _contentTypeMap["h"] = "text/x-c";
    _contentTypeMap["hpp"] = "text/x-c++";
}

std::string ContentTypeManager::getContentType(const std::string& filename) {
    std::string extension = extractExtension(filename);
    if (_contentTypeMap.find(extension) != _contentTypeMap.end()) {
        return _contentTypeMap[extension];
    }
    return "application/octet-stream";
}

std::string ContentTypeManager::getExtension(const std::string& content_type) {
    if (content_type.empty()) {
        return "";
    }
    for (ContentTypeMap::const_iterator it = _contentTypeMap.begin(); it != _contentTypeMap.end(); ++it) {
        if (it->second == content_type) {
            return it->first;
        }
    }
    return "";
}

}  // namespace http
