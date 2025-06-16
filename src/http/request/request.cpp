#include "request.hpp"
#include "io_pending_state.hpp"

namespace http {
void Request::run() {
    const std::size_t root_depth = 0;
    switch (_ioPendingState) {
        case START_READING:
        case NO_IO_PENDING:
        case REQUEST_READING:
            if (_requestDepth == root_depth) {
                recvRequest();
                if (_ioPendingState != NO_IO_PENDING && _ioPendingState != RESPONSE_START)
                    break;
            }
        // fallthrough
        case CGI_BODY_SENDING:
        case CGI_OUTPUT_READING:
        case CGI_LOCAL_REDIRECT_IO_PENDING:
            handleRequest();
            if (_ioPendingState != NO_IO_PENDING && _ioPendingState != RESPONSE_START)
                break;
        // fallthrough
        case ERROR_LOCAL_REDIRECT_IO_PENDING:
        case RESPONSE_START:
        case RESPONSE_SENDING:
            if (_requestDepth == root_depth || _requestDepth == http::cgi::MAX_REDIRECTS) {
                Request::sendResponse();
            }
            break;
        default:
            // never come here
            break;
    }
}

}  // namespace http
