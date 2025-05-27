#include "request.hpp"

#include <string>
#include <vector>

#include "../../../toolbox/access.hpp"
#include "../../../toolbox/string.hpp"

namespace {
    std::string getFieldValue(
        http::HTTPRequest& request, const std::string& fieldName) {
        std::vector<std::string> values = request.fields.getFieldValue(fieldName);
        if (!values.empty()) {
            return values[0];
        }
        return "-";
    }

}

void http::Request::sendResponse() {
    int status = _response.getStatus();

    std::string remote_addr = _client->getIp();
    std::string remote_user = "-";
    std::string request = _parsedRequest.get().originalRequestLine;
    std::size_t body_bytes_sent = _response.getContentLength();
    std::string http_referer = getFieldValue(
        _parsedRequest.get(), "Referer");
    std::string http_user_agent = getFieldValue(
        _parsedRequest.get(), "User-Agent");

    toolbox::logger::AccessLog::log(
        remote_addr,
        remote_user,
        request,
        status,
        body_bytes_sent,
        http_referer,
        http_user_agent
    );
    _response.sendResponse(_client->getFd());
}