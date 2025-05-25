#include <string>

#include "../../../toolbox/stepmark.hpp"
#include "../../core/client.hpp"
#include "request_parser.hpp"
#include "request.hpp"

namespace http {

namespace {
const std::size_t BUFFER_SIZE = 2048;
}

bool Request::recvRequest() {
    char buffer[BUFFER_SIZE];
    int bytesReceived = 0;

    bytesReceived = recv(_client->getFd(), buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0) {
        toolbox::logger::StepMark::info("Request: recvRequest: client "
            "disconnected " + toolbox::to_string(_client->getFd()));
        if (_parsedRequest.getValidatePos() == BaseParser::V_COMPLETED) {
            _response.setStatus(HttpStatus::OK);
            return true;
        } else {
            _response.setStatus(HttpStatus::BAD_REQUEST);
            return false;
        }
    } else if (bytesReceived == -1) {
        toolbox::logger::StepMark::error("Request: recvRequest: recv failed "
            "in recv from " + toolbox::to_string(_client->getFd()));
        _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        return false;
    }

    buffer[bytesReceived] = '\0';

    _parsedRequest.run(static_cast<std::string>(buffer));

    std::size_t clientMaxBodySize = 0;

    BaseParser::ValidatePos validatePos = _parsedRequest.getValidatePos();
    if (validatePos == BaseParser::V_BODY ||
        validatePos == BaseParser::V_COMPLETED ||
        _parsedRequest.get().body.contentLength != std::size_t(-1)) {
        if (_config.getPath().empty()) {  // If path is empty, fetch the config
            fetchConfig();
        }

        // Get client max body size once and reuse
        clientMaxBodySize = _config.getClientMaxBodySize();

        std::size_t contentLength = _parsedRequest.get().body.contentLength;
        if (contentLength > clientMaxBodySize) {
            toolbox::logger::StepMark::info(
                "Request: recvRequest: content length exceeds "
                "client max body size");
            _response.setStatus(HttpStatus::PAYLOAD_TOO_LARGE);
            return false;
        }

        std::size_t receivedLength = _parsedRequest.get().body.receivedLength;
        if (receivedLength > clientMaxBodySize) {
            toolbox::logger::StepMark::info(
                "Request: recvRequest: receivedLength exceeds "
                "client max body size");
            _response.setStatus(HttpStatus::PAYLOAD_TOO_LARGE);
            return false;
        }
    }
    return true;
}

}  // namespace http
