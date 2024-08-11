#include "stringFromChar.hpp"


std::string stringFromChar(unsigned char* text, long textLength) {
  // Sometimes textLength is the actual length. Other times it's
  // a value that indicates the string is null terminated
  long inputLength =
      (textLength == CHAR_IS_NTS)
          ? static_cast<long>(strlen(reinterpret_cast<const char*>(text)))
          : textLength;

  return std::string(reinterpret_cast<char*>(text), inputLength);
}

std::string stringFromChar(char* text, long textLength) {
  // Sometimes textLength is the actual length. Other times it's
  // a value that indicates the string is null terminated
  long inputLength =
      (textLength == CHAR_IS_NTS)
          ? static_cast<long>(strlen(reinterpret_cast<const char*>(text)))
          : textLength;

  return std::string(reinterpret_cast<char*>(text), inputLength);
}
