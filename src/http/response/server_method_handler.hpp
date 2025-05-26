#pragma once

#include <string>
#include <vector>
#include <map>

#include "../request/http_fields.hpp"
#include "../case_insensitive_less.hpp"
#include "../http_status.hpp"
#include "../request/request_parser.hpp"
#include "content_type_manager.hpp"
#include "method_utils.hpp"
#include "response.hpp"

namespace config {
class LocationConfig;  // Forward declaration
}

namespace http {
namespace serverMethod {
void serverMethodHandler(RequestParser& parsedRequest,
                         const config::LocationConfig& config,
                         HTTPFields& fields,
                         Response& response);

void runGet(const std::string& targetPath, std::vector<std::string> indices,
    bool isAutoindex, Response& response);
void runHead(const std::string& targetPath, std::vector<std::string> indices,
    bool isAutoindex, Response& response);
void runDelete(const std::string& path, Response& response);
void runPost(const std::string& uploadPath, std::string& recvBody,
    HTTPFields& fields, Response& response);

}  // namespace serverMethod
}  // namespace http
