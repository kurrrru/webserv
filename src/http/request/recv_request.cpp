#include <string>
#include <limits>

#include "../../../toolbox/stepmark.hpp"
#include "../../core/client.hpp"
#include "../../core/constant.hpp"
#include "request_parser.hpp"
#include "request.hpp"
#include "io_pending_state.hpp"

namespace http {
namespace {

int handleRecvResult(int receivedSize,
                      BaseParser::ValidatePos validatePos,
                      const toolbox::SharedPtr<Client>& client) {
    int statusCode = 200;
    if (receivedSize == 0) {
        toolbox::logger::StepMark::info("Request: recvRequest: client "
            "disconnected " + toolbox::to_string(client->getFd()));
        if (validatePos != BaseParser::V_COMPLETED) {
            statusCode = 400;
        }
    } else if (receivedSize == -1) {
        toolbox::logger::StepMark::error("Request: recvRequest: recv failed "
            "in recv from " + toolbox::to_string(client->getFd()));
        statusCode = 500;
    }
    return statusCode;
}

bool isValidContentLength(std::size_t contentLength,
                          std::size_t clientMaxBodySize) {
    if (contentLength != std::numeric_limits<std::size_t>::max()
        && contentLength > clientMaxBodySize) {
        toolbox::logger::StepMark::info( "Request: recvRequest: content length"
            " exceeds client max body size");
        return false;
    }
    return true;
}

bool isValidReceivedLength(std::size_t receivedLength,
                           std::size_t clientMaxBodySize) {
    if (receivedLength > clientMaxBodySize) {
        toolbox::logger::StepMark::info("Request: recvRequest: received length"
            " exceeds client max body size");
        return false;
    }
    return true;
}

}  // namespace

bool Request::performRecv(std::string& receivedData) {
    char buffer[core::IO_BUFFER_SIZE];

    int receivedSize = recv(_client->getFd(), buffer, core::IO_BUFFER_SIZE, 0);
    int statusCode = handleRecvResult(receivedSize, 
                                  _parsedRequest.getValidatePos(), _client);

    if (statusCode != 200) {
        _response.setStatus(statusCode);
        return false;
    }

    receivedData = std::string(buffer, receivedSize);
    return true;
}

bool Request::loadConfig() {
    fetchConfig();
    if (_response.getStatus() != HttpStatus::OK) {
        toolbox::logger::StepMark::info(
            "Request: loadConfig: fetchConfig failed");
        return false;
    }
    return true;
}

bool Request::isValidBodySize() {
    const BaseParser::ValidatePos validatePos = _parsedRequest.getValidatePos();
    if (validatePos != BaseParser::V_BODY &&
        validatePos != BaseParser::V_COMPLETED) {
        return true;
    }

    if (!loadConfig()) {
        return false;
    }

    const std::size_t clientMaxBodySize = _config.getClientMaxBodySize();
    const std::size_t contentLength = _parsedRequest.get().body.contentLength;
    const std::size_t receivedLength = _parsedRequest.get().body.receivedLength;

    if (!isValidContentLength(contentLength, clientMaxBodySize) ||
        !isValidReceivedLength(receivedLength, clientMaxBodySize)) {
        return false;
    }
    return true;
}

bool Request::recvRequest() {
    std::string receivedData;

    if (!performRecv(receivedData)) {
        _ioPendingState = NO_IO_PENDING;
        toolbox::logger::StepMark::error("Request: recvRequest: failed to receive data");
        return false;
    }

    if (_ioPendingState == START_READING && (receivedData == symbols::CRLF || receivedData == symbols::LF)) {
        return true;
    }

    _ioPendingState = REQUEST_READING;

    int parseStatus = _parsedRequest.run(receivedData);
    
    if (parseStatus == BaseParser::P_ERROR) {
        _response.setStatus(_parsedRequest.get().httpStatus.get());
        _ioPendingState = NO_IO_PENDING;
        toolbox::logger::StepMark::error("Request: recvRequest: failed to parse request");
        return false;
    }
    
    if (!isValidBodySize()) {
        _response.setStatus(HttpStatus::PAYLOAD_TOO_LARGE);
        _ioPendingState = NO_IO_PENDING;
        toolbox::logger::StepMark::error("Request: recvRequest: request have "
            "invalid body/content size");
            return false;
        }

    if (parseStatus == BaseParser::P_COMPLETED) {
        _ioPendingState = NO_IO_PENDING;
        toolbox::logger::StepMark::info("Request: recvRequest: request "
            "received and parsed successfully");
    }
    return true;
}

}  // namespace http
