// Copyright 2025 Ideal Broccoli

#pragma once

namespace http {
enum IOPendingState {
    START_READING,
    REQUEST_READING,
    CGI_BODY_SENDING,
    CGI_OUTPUT_READING,
    CGI_LOCAL_REDIRECT_IO_PENDING,
    ERROR_LOCAL_REDIRECT_IO_PENDING,
    RESPONSE_START,
    RESPONSE_SENDING,
    NO_IO_PENDING,
    END_RESPONSE
};
}  // namespace http
