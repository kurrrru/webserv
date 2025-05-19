#pragma once

#include <map>
#include <string>

#include "method_utils.hpp"

namespace http {

typedef std::map<std::string, std::string> ContentTypeMap;

class ContentTypeManager {
 public:
    static std::string getContentType(const std::string& filename);
    static std::string getExtension(const std::string& filename);
 private:
    static void initContentTypeMap();
    static ContentTypeMap _contentTypeMap;
    ContentTypeManager();
    ~ContentTypeManager();
    ContentTypeManager(const ContentTypeManager&);
    ContentTypeManager& operator=(const ContentTypeManager&) { return *this; };
};

}  // namespace http
