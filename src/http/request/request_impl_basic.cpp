#include "request.hpp"

#include "../../core/client.hpp"
#include "../../../toolbox/shared.hpp"

http::Request::Request(const toolbox::SharedPtr<Client>& client, std::size_t requestDepth)
    : _client(client), _requestDepth(requestDepth) {
}

http::Request::~Request() {
}
