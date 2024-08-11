#include "trinoExceptions.hpp"

AuthError::AuthError(std::string& message) : std::runtime_error(message) {};

AuthError::AuthError(const char* message) : std::runtime_error(message) {};
