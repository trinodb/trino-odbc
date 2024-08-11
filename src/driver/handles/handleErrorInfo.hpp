#pragma once

#include <string>

class ErrorInfo {
  public:
    ErrorInfo();
    ErrorInfo(std::string errorMessage, std::string sqlStateCode);
    std::string errorMessage = "";
    std::string sqlStateCode = "";
    bool errorOccurred();
};
