#include "b64decoder.hpp"
#include <algorithm>
#include <unordered_map>
#include <utility>

#include "windowsLean.hpp"
#include <wincrypt.h>

#include "writeLog.hpp"

std::string fromBase64url(const std::string& input) {
  /*
  NOTE: base64url encoding is different from base64 encoding.
  This is an implementation of the former.
  */
  DWORD requiredSize = 0;
  // Calling with a null size pointer causes the function to calculate the
  // required output size instead of encrypting the string.
  if (!CryptStringToBinary(input.c_str(),
                           0,
                           CRYPT_STRING_BASE64URI,
                           NULL,
                           &requiredSize,
                           NULL,
                           NULL)) {
    std::string errorMessage = "Error calculating base64url decoding size";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  std::vector<BYTE> decodedData(requiredSize);
  if (!CryptStringToBinary(input.c_str(),
                           0,
                           CRYPT_STRING_BASE64,
                           decodedData.data(),
                           &requiredSize,
                           NULL,
                           NULL)) {
    std::string errorMessage = "Error decoding from base64url";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  return std::string(reinterpret_cast<char*>(decodedData.data()), requiredSize);
}
