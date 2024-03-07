#pragma once

#include <exception>

// Bare exception implementation to be able to signal a socket timeout
class SocketTimeoutException : std::exception {
public:
    const char* what() const noexcept override {
        return "socket timeout";
    }
};
