#pragma once

#include <string>

#include "../../../toolbox/string.hpp"
#include "../request/http_fields.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "response.hpp"
#include "method_utils.hpp"
#include "content_type_manager.hpp"

namespace http {
struct FormDataField {
    std::string filename;
    std::string content;
};

void runPost(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields, Response& response);
}
