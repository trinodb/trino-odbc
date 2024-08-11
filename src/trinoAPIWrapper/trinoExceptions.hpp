#pragma once

#include <stdexcept>
#include <string>

class AuthError : public std::runtime_error {
  public:
    explicit AuthError(std::string& message);
    explicit AuthError(const char* message);
};
