#include "request.hpp"

#include <string>
#include <vector>

#include "../../core/client.hpp"
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

    void propagateErrorPage(
        http::Response* response, const http::Response& errorResponse) {
        typedef std::string FieldName;
        typedef std::string FieldDefaultValue;

        // [TODO] ここも後で確認します
        // const std::pair<std::string, std::string> defaultHeadersArray[] = {
        //     {http::fields::CONTENT_TYPE, "text/plain"},
        //     {http::fields::CACHE_CONTROL, "no-cache"},
        //     {http::fields::CONNECTION, "close"},
        //     {http::fields::SERVER, "WebServer/1.0"}
        // };

        const std::pair<std::string, std::string> defaultHeadersArray[] = {
            std::make_pair(http::fields::CONTENT_TYPE, "text/plain"),
            std::make_pair(http::fields::CACHE_CONTROL, "no-cache"),
            std::make_pair(http::fields::CONNECTION, "close"),
            std::make_pair(http::fields::SERVER, "WebServer/1.0")
        };

        const std::vector<std::pair<FieldName, FieldDefaultValue> > defaultHeaders(
            defaultHeadersArray, defaultHeadersArray + sizeof(defaultHeadersArray)
            / sizeof(defaultHeadersArray[0]));

        for (std::size_t i = 0; i < defaultHeaders.size(); ++i) {
            const std::pair<FieldName, FieldDefaultValue>& header =
                defaultHeaders[i];
            std::string headerValue = errorResponse.getHeader(header.first);
            if (headerValue.empty()) {
                headerValue = header.second;
            }
            response->setHeader(header.first, headerValue);
        }
        response->setBody(errorResponse.getBody());
    }

    void setDefaultErrorPage(http::Response* response, std::size_t status) {
        // [TODO] 設定すべきフィールドを確認する
        // [TODO] デフォルトのエラーページを設定する
        response->setHeader(http::fields::CONTENT_TYPE, "text/html");
        response->setStatus(status);
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head>\n"
            << "<title>Error " << status << "</title>\n"
            << "</head>\n"
            << "<body>\n"
            << "<h1>Error " << status << "</h1>\n"
            << "<p>An error occurred while processing your request.</p>\n"
            << "</body>\n"
            << "</html>";
        response->setBody(ss.str());
    }

}

void http::Request::sendResponse() {
    std::size_t status = _response.getStatus();

    if (status >= config::directive::MIN_ERROR_PAGE_CODE
            && status <= config::directive::MAX_ERROR_PAGE_CODE) {
        std::vector<config::ErrorPage> errorPages = _config.getErrorPages();
        bool useDefaultErrorPage = true;
        for (std::size_t i = 0; i < errorPages.size(); ++i) {
            if (std::find(errorPages[i].getCodes().begin(),
                    errorPages[i].getCodes().end(), status)
                != errorPages[i].getCodes().end()) {
                // [TODO] Change the processing of the error page
                // depending on the prefix of the error page path
                // "/" - ngx_http_internal_redirect
                // "@" - ngx_http_named_location
                // otherwise - ngx_http_send_refresh or ngx_http_send_special_response

                // [TODO] この辺は後で書きます

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
                // errorRequest.setLocalRedirectInfo(method, path, host);
                errorRequest.fetchConfig();
                errorRequest.handleRequest();
                if (errorRequest._response.getStatus() == http::HttpStatus::OK) {
                    propagateErrorPage(&_response, errorRequest._response);
                    useDefaultErrorPage = false;
                }
            }
        }
        if (useDefaultErrorPage) {
            setDefaultErrorPage(&_response, status);
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