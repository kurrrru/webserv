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
    std::size_t status = _response.getStatus();

    if (status >= config::directive::MIN_ERROR_PAGE_CODE
            && status <= config::directive::MAX_ERROR_PAGE_CODE) {
        std::vector<config::ErrorPage> errorPages = _config.getErrorPages();
        for (std::size_t i = 0; i < errorPages.size(); ++i) {
            if (std::find(errorPages[i].getCodes().begin(),
                    errorPages[i].getCodes().end(), status)
                != errorPages[i].getCodes().end()) {
                http::Request errorRequest(_client, _requestDepth + 1);
                std::string method = http::method::GET;
                std::string path = errorPages[i].getPath();
                std::string host;
                if (_parsedRequest.get().fields.getFieldValue(
                        http::fields::HOST).empty()) {
                    host = _client->getServerIp();
                } else {
                    host = _parsedRequest.get().fields.getFieldValue(
                        http::fields::HOST)[0];
                }
                errorRequest.setLocalRedirectInfo(method, path, host);
                errorRequest.fetchConfig();
                errorRequest.handleRequest();
                if (errorRequest._response.getStatus() == http::HttpStatus::OK) {
                    // content-type も設定する必要あり
                    _response.setBody(errorRequest._response.getBody());
                } else {
                    // _response.setHeader(http::fields::CONTENT_TYPE,
                    //     "text/html");
                    // _response.setBody("<html><body>"
                    //     "<h1>Error: " + std::to_string(status) + "</h1>"
                    //     "<p>Internal error occurred.</p></body></html>");
                }
            }
        }
    } 



    std::string remote_addr = _client->getIp();
    std::string remote_user = "-";
    std::string request = _parsedRequest.get().originalRequestLine;
    std::size_t body_bytes_sent = _response.getContentLength();
    std::string http_referer = getFieldValue(
        _parsedRequest.get(), http::fields::REFERER);
    std::string http_user_agent = getFieldValue(
        _parsedRequest.get(), http::fields::USER_AGENT);

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