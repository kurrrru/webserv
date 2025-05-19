#pragma once

#include <string>
#include <map>

#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../request/http_fields.hpp"
#include "content_type_manager.hpp"
#include "method_utils.hpp"
#include "response.hpp"

namespace http {
HttpStatus::EHttpStatus runHead(const std::string& path,
                               const std::string& indexPath,
                               bool isAutoindex,
                               Response& response);
}  // namespace http
