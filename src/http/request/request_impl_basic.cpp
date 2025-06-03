#include "request.hpp"

#include "../../core/client.hpp"
#include "../../../toolbox/shared.hpp"
#include "request_parser.hpp"

http::Request::Request(const toolbox::SharedPtr<Client>& client, std::size_t requestDepth)
    : _client(client), _requestDepth(requestDepth), _ioPendingState(REQUEST_READING) {
}

http::Request::~Request() {
}

bool http::Request::isKeepAliveRequest() const {
    return _parsedRequest.isKeepAlive();
}
