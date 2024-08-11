#include "handleErrorInfo.hpp"

ErrorInfo::ErrorInfo() {}

ErrorInfo::ErrorInfo(std::string errorMessage, std::string sqlStateCode) {
  this->errorMessage = errorMessage;
  this->sqlStateCode = sqlStateCode;
}

bool ErrorInfo::errorOccurred() {
  // We'll use the presence of a sql state code to
  // indicate if this is an actual error or just
  // a placeholder error info object.
  return not this->sqlStateCode.empty();
}
