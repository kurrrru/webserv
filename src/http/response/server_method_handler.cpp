#include <string>
#include <vector>

#include "../../config/config_base.hpp"
#include "../../config/config_http.hpp"
#include "../../config/config_server.hpp"
#include "../../config/config_location.hpp"
#include "server_method_handler.hpp"

namespace http {
namespace serverMethod {
void serverMethodHandler(RequestParser& parsedRequest,
                         const config::LocationConfig &config,
                         HTTPFields& fields,
                         Response& response) {
    std::string rootPath = config.getRoot();
    std::string fullPath = joinPath(rootPath, parsedRequest.get().uri.path);
    std::string method = parsedRequest.get().method;
    std::vector<std::string> indices = config.getIndices();
    bool isAutoindex = config.getAutoindex();

    if (method == method::GET) {
        runGet(fullPath, indices, isAutoindex, response);
    } else if (method == method::HEAD) {
        runHead(fullPath, indices, isAutoindex, response);
    } else if (method == method::DELETE) {
        runDelete(fullPath, response);
    } else if (method == method::POST) {
        std::string serverUploadsPath = joinPath(rootPath, config.getPath());
        std::string uploadStorePath = joinPath(serverUploadsPath, config.getUploadStore());
        std::string recvBody = parsedRequest.get().body.content;
        runPost(uploadStorePath, recvBody, fields, response);
    }
}

}  // namespace serverMethod
}  // namespace http
