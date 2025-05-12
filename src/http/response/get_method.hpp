#pragma once

#include <string>
#include <map>
#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"

namespace http {

typedef std::map<std::string, std::string, CaseInsensitiveLess> ExtensionMap;

void initExtensionMap(ExtensionMap& extensionMap);
HttpStatus::EHttpStatus runGet(const std::string& path,
                             std::string& responseBody,
                             const std::string& indexPath,
                             bool isAutoindex,
                             std::string& contentType,
                             ExtensionMap& extensionMap);
void readDirectoryEntries(const std::string& dirPath,
                          std::string& responseBody);

}  // namespace http
