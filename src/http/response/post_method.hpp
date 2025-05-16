#pragma once

#include <string>

#include "../../../toolbox/string.hpp"
#include "../request/http_fields.hpp"
#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "response.hpp"
#include "method_utils.hpp"

namespace http {
struct FormDataField {
    std::string name;
    std::string filename;
    std::string content_type;
};

void runPost(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields, Response& response);
}
