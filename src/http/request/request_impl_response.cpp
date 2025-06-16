#include "request.hpp"

#include <string>
#include <vector>

#include "../../core/client.hpp"
#include "../../../toolbox/access.hpp"
#include "../../../toolbox/string.hpp"
#include "io_pending_state.hpp"

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
        response->setStatus(status);
        response->setHeader(http::fields::CONTENT_TYPE, "text/html");
        std::stringstream ss;

        ss << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head>\n"
            << "<title>Error " << status << "</title>\n"
            << "</head>\n"
            << "<body>\n"
            << "<p><strong>Error " << status << "</strong></p>\n"
            << "<p>\n"
            << "The essence you seek, in its most harmonious form, "
            << "resides within the realm of Ideals.<br>\n"
            << "What we encounter now is but a fleeting shadow, "
            << "a temporary deviation from its true perfection.<br>\n"
            << "Fear not, for just like the Ideal Broccoli, "
            << "always striving for its perfect green, "
            << "its ultimate manifestation will surely emerge.<br>"
            << "<small>- Ideal Broccoli: Cultivating perfection, one step at a time.</small>\n"
            << "</p>"
            << "</body>\n"
            << "</html>\n";
        response->setBody(ss.str());
    }

}

void http::Request::sendResponse() {
    if (_ioPendingState == http::END_RESPONSE) {
        return;
    }
    std::size_t status = _response.getStatus();
    if (_ioPendingState != http::RESPONSE_SENDING && 
        status >= config::directive::MIN_ERROR_PAGE_CODE && 
        status <= config::directive::MAX_ERROR_PAGE_CODE) {
        if (_ioPendingState != http::ERROR_LOCAL_REDIRECT_IO_PENDING) {
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

                    _errorPageRequest = toolbox::SharedPtr<http::Request>(
                        new http::Request(_client, _requestDepth + 1));
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
                    _errorPageRequest->setLocalRedirectInfo(method, path, host);
                    _errorPageRequest->fetchConfig();
                    useDefaultErrorPage = false;
                    break;
                }
            }
            if (useDefaultErrorPage) {
                setDefaultErrorPage(&_response, status);
            }
        }
        if (_errorPageRequest) {
            _errorPageRequest->run();
            if (_errorPageRequest->getIOPendingState() == http::CGI_BODY_SENDING
                || _errorPageRequest->getIOPendingState() == http::CGI_OUTPUT_READING
                || _errorPageRequest->getIOPendingState() == http::CGI_LOCAL_REDIRECT_IO_PENDING) {
                _ioPendingState = http::ERROR_LOCAL_REDIRECT_IO_PENDING;
                return;
            }
            int errorStatus = _errorPageRequest->getResponse().getStatus();

            const int MIN_SUCCESS_CODE = 200;
            const int MAX_SUCCESS_CODE = 399;
            if (errorStatus >= MIN_SUCCESS_CODE && errorStatus <= MAX_SUCCESS_CODE) {
                propagateErrorPage(&_response, _errorPageRequest->getResponse());
            } else {
                setDefaultErrorPage(&_response, status);
            }
        }
    }

    if (_ioPendingState != http::RESPONSE_SENDING) {
        // access logはio_pending_stateがRESPONSE_SENDINGに変更すると同時に記録する
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
        _ioPendingState = http::RESPONSE_SENDING;
        return;
    }
    bool endSending = _response.sendResponse(_client->getFd());
    if (endSending) {
        _ioPendingState = http::END_RESPONSE;
    }
}