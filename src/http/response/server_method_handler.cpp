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
    std::string targetPath = parsedRequest.get().uri.path;
    std::string rootPath = config.getRoot();
    targetPath = joinPath(rootPath, targetPath);
    toolbox::logger::StepMark::info("serverMethodHandler: make targetPath = " + targetPath);

    std::string method = parsedRequest.get().method;
    std::vector<std::string> indices = config.getIndices();
    bool isAutoindex = config.getAutoindex();

    if (method == method::GET) {
        runGet(targetPath, indices, isAutoindex, response);
    } else if (method == method::HEAD) {
        runHead(targetPath, indices, isAutoindex, response);
    } else if (method == method::DELETE) {
        runDelete(targetPath, response);
    } else if (method == method::POST) {
        std::string serverUploadsPath = joinPath(config.getRoot(), config.getPath());
        std::string uploadStorePath = joinPath(serverUploadsPath, config.getUploadStore());
        std::string recvBody = parsedRequest.get().body.content;
        runPost(uploadStorePath, recvBody, fields, response);
    }
}

}  // namespace serverMethod
}  // namespace http
