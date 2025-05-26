#include <string>
#include <vector>

#include "server_method_handler.hpp"

namespace http {
namespace serverMethod {
void serverMethodHandler(RequestParser& parsedRequest,
                         const config::LocationConfig &config,
                         HTTPFields fields,
                         Response& response) {
    std::string method = parsedRequest.get().method;
    std::string targetPath = parsedRequest.get().uri.path;
    std::vector<std::string> indices = config.getIndices();
    bool isAutoindex = config.getAutoindex();

    if (method == method::GET) {
        runGet(targetPath, indices, isAutoindex, response);
    } else if (method == method::HEAD) {
        runHead(targetPath, indices, isAutoindex, response);
    } else if (method == method::DELETE) {
        runDelete(targetPath, response);
    } else if (method == method::POST) {
        std::string uploadPath = config.getUploadStore();
        std::string recvBody = parsedRequest.get().body.content;
        runPost(uploadPath, recvBody, fields, response);
    }
}

}  // namespace serverMethod
}  // namespace http
