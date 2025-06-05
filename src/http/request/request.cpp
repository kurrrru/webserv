#include "request.hpp"
#include "io_pending_state.hpp"

namespace http {
void Request::run() {
  switch (_ioPendingState) {
    case NO_IO_PENDING:
    case REQUEST_READING:
        recvRequest();
        if (_ioPendingState != NO_IO_PENDING)
            break;
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
        Request::sendResponse();
        break;
    default:
        // never come here
        break;
  }
}

}  // namespace http
