#include <string>

#include "../../../toolbox/stepmark.hpp"
#include "../../core/client.hpp"
#include "request_parser.hpp"
#include "request.hpp"

namespace http {

bool Request::recvRequest() {
    char buffer[2048];
    int bytesReceived = 0;

    bytesReceived = recv(_client->getFd(), buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0) {
        toolbox::logger::StepMark::info("Request: recvRequest: client disconnected " + _client->getFd());
        if (_parsedRequest.getValidatePos() == BaseParser::V_COMPLETED) {
            _response.setStatus(HttpStatus::OK);
            return true;
        } else {
            _response.setStatus(HttpStatus::BAD_REQUEST);
            return false;
        }
    } else if (bytesReceived == -1) {
        toolbox::logger::StepMark::error("Request: recvRequest: recv failed in recv from " + _client->getFd());
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }

    buffer[bytesReceived] = '\0';
    
    _parsedRequest.run(static_cast<std::string>(buffer));

    std::size_t clientMaxBodySize = 0;

    if (_parsedRequest.getValidatePos() == BaseParser::V_BODY ||
        _parsedRequest.getValidatePos() == BaseParser::V_COMPLETED) {
        if (_config.getPath().empty()) {  // If path is empty, fetch the config
            fetchConfig();

            std::size_t contentLength = _parsedRequest.get().body.contentLength;
            clientMaxBodySize = _config.getClientMaxBodySize();
            if (contentLength > clientMaxBodySize) {
                toolbox::logger::StepMark::info("Request: recvRequest: content length exceeds client max body size");
                _response.setStatus(HttpStatus::PAYLOAD_TOO_LARGE);
                return false;
            }
        }

        std::size_t recievedLength = _parsedRequest.get().body.recvedLength;
        clientMaxBodySize = _config.getClientMaxBodySize();
        if (recievedLength > clientMaxBodySize) {
            toolbox::logger::StepMark::info("Request: recvRequest: recievedLength exceeds client max body size");
            _response.setStatus(HttpStatus::PAYLOAD_TOO_LARGE);
            return false;
        }
    }
    return true;
}

}  // namespace http
