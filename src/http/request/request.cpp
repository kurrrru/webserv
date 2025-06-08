#include "request.hpp"
#include "io_pending_state.hpp"

namespace http {
void Request::run() {
    const std::size_t root_depth = 0;
    switch (_ioPendingState) {
        case NO_IO_PENDING:
        case REQUEST_READING:
            if (_requestDepth == root_depth) {
                recvRequest();
                if (_ioPendingState != NO_IO_PENDING)
                    break;
            }
        // fallthrough
        case CGI_BODY_SENDING:
        case CGI_OUTPUT_READING:
        case CGI_LOCAL_REDIRECT_IO_PENDING:
            handleRequest();
            if (_ioPendingState != NO_IO_PENDING)
                break;
        // fallthrough
        case ERROR_LOCAL_REDIRECT_IO_PENDING:
        case RESPONSE_SENDING:
            if (_requestDepth == root_depth) {
                Request::sendResponse();
            }
            break;
        default:
            // never come here
            break;
    }
}

}  // namespace http
