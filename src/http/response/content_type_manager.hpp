#pragma once

#include <map>
#include <string>

#include "method_utils.hpp"

namespace http {

typedef std::map<std::string, std::string> ContentTypeMap;

class ContentTypeManager {
 public:
    static ContentTypeManager& getInstance() {
        static ContentTypeManager instance;
        return instance;
    }
    std::string getContentType(const std::string& filename);
    std::string getExtension(const std::string& filename);

 private:
    static ContentTypeMap _contentTypeMap;

    ContentTypeManager();
    ~ContentTypeManager() {};
    ContentTypeManager(const ContentTypeManager&);
    ContentTypeManager& operator=(const ContentTypeManager&) { return *this; };
};

}  // namespace http
