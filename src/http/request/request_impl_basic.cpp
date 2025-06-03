#include <string>
#include "request.hpp"

#include "../../../toolbox/shared.hpp"

http::Request::Request(const toolbox::SharedPtr<Client>& client,
                        std::size_t requestDepth)
    : _client(client), _requestDepth(requestDepth) {
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
