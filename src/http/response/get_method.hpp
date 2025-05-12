#pragma once

#include <string>
#include <map>
#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "method_utils.hpp"

namespace http {
HttpStatus::EHttpStatus runGet(const std::string& path,
                               std::string& responseBody,
                               const std::string& indexPath,
                               bool isAutoindex,
                               std::string& contentType,
                             ExtensionMap& extensionMap);
void readDirectoryEntries(const std::string& dirPath,
                          std::string& responseBody);

}  // namespace http
