#include <string>
#include "request.hpp"

#include "../../core/client.hpp"
#include "../../../toolbox/shared.hpp"
#include "request_parser.hpp"
#include "io_pending_state.hpp"

http::Request::Request(const toolbox::SharedPtr<Client>& client, std::size_t requestDepth)
    : _client(client), _requestDepth(requestDepth), _ioPendingState(REQUEST_READING) {
}

http::Request::~Request() {
}

void http::Request::setLocalRedirectInfo(const std::string& method,
                                        const std::string& path,
                                        const std::string& host) {
    _parsedRequest.get().method = method;
    _parsedRequest.get().uri.path = path;
    _parsedRequest.get().fields.getFieldValue(fields::HOST).push_back(host);
}

http::Response http::Request::getResponse() const {
    return _response;
}

void http::Request::setRedirectCount(size_t count) {
    _requestDepth = count;
    _cgiHandler.setRedirectCount(count);
}

const std::string& http::Request::getUploadPath() const {
    return _config.getUploadStore();
}
