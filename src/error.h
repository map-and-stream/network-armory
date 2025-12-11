#pragma once

#include <string>

enum class ErrorCode {
    NO_ERROR = 0,
    CONNECTION_FAILED = 1,
    DISCONNECTED = 2,
    SEND_FAILED = 3,
    RECEIVE_FAILED = 4,
    TIMEOUT = 5,
    INVALID_ADDRESS = 6,
    UNSUPPORTED_PROTOCOL = 7,
    SSL_ERROR = 8,
    CONFIGURATION_ERROR = 9,
    ALREADY_CONNECTED = 10,
    NOT_CONNECTED = 11,
    INTERNAL_ERROR = 12,

    // Added for UDP server + client behavior
    PORT_IN_USE = 13,         // Server cannot bind to port
    SERVER_UNAVAILABLE = 14,  // Client cannot reach server
    NOT_IMPLEMENTED = 15,
    DISCONNECTION_FAILED = 16,
};

inline std::string error_message_from_code(ErrorCode code) {
    switch (code) {
        case ErrorCode::NO_ERROR:
            return "No error";
        case ErrorCode::CONNECTION_FAILED:
            return "Connection failed";
        case ErrorCode::DISCONNECTED:
            return "Disconnected";
        case ErrorCode::SEND_FAILED:
            return "Send failed";
        case ErrorCode::RECEIVE_FAILED:
            return "Receive failed";
        case ErrorCode::TIMEOUT:
            return "Timeout";
        case ErrorCode::INVALID_ADDRESS:
            return "Invalid address";
        case ErrorCode::UNSUPPORTED_PROTOCOL:
            return "Unsupported protocol";
        case ErrorCode::SSL_ERROR:
            return "SSL error";
        case ErrorCode::CONFIGURATION_ERROR:
            return "Configuration error";
        case ErrorCode::ALREADY_CONNECTED:
            return "Already connected";
        case ErrorCode::NOT_CONNECTED:
            return "Not connected";
        case ErrorCode::INTERNAL_ERROR:
            return "Internal error";

        // New messages
        case ErrorCode::PORT_IN_USE:
            return "Port is already in use";
        case ErrorCode::SERVER_UNAVAILABLE:
            return "Server is unavailable";

        default:
            return "Unknown error";
    }
}

struct Error {
    ErrorCode error_code = ErrorCode::NO_ERROR;
    std::string error_message;

    Error* set_code(ErrorCode e) noexcept {
        error_code = e;
        return this;
    }

    Error* set_message(const std::string& m) {
        error_message = m;
        return this;
    }

    ErrorCode code() const { return error_code; }
    const std::string& message() const { return error_message; }

    std::string to_string() const {
        return "code[" + std::to_string(static_cast<int>(error_code)) + "] message[" +
               error_message_from_code(error_code) + "]";
    }

    void clear() {
        error_code = ErrorCode::NO_ERROR;
        error_message.clear();
    }
};
