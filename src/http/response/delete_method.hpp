#include <string>

#include <cstring>

#include "../http_status.hpp"
#include "../http_namespace.hpp"
#include "response.hpp"
#include "method_utils.hpp"

namespace http {
void runDelete(const std::string& path, Response& response);
}
