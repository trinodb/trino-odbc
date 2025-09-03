#include "cryptUtils.hpp"

#include <stdexcept>
#include <vector>

#include "windowsLean.hpp"
#include <dpapi.h>
#include <wincrypt.h>

#include "writeLog.hpp"

std::string toBase64(const std::vector<BYTE>& data) {
  DWORD requiredSize = 0;
  // Calling with a null size pointer causes the function to calculate the
  // required output size instead of encrypting the string.
  if (!CryptBinaryToString(data.data(),
                           static_cast<DWORD>(data.size()),
                           CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                           NULL,
                           &requiredSize)) {
    std::string errorMessage = "Error calculating base64 encoding size";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  std::string encodedString(requiredSize, '\0');
  if (!CryptBinaryToString(data.data(),
                           static_cast<DWORD>(data.size()),
                           CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                           encodedString.data(),
                           &requiredSize)) {
    std::string errorMessage = "Error encoding to base64";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  return encodedString;
}

std::vector<BYTE> fromBase64(const std::string& data) {
  DWORD requiredSize = 0;
  // Calling with a null size pointer causes the function to calculate the
  // required output size instead of encrypting the string.
  if (!CryptStringToBinary(data.c_str(),
                           0,
                           CRYPT_STRING_BASE64,
                           NULL,
                           &requiredSize,
                           NULL,
                           NULL)) {
    std::string errorMessage = "Error calculating base64 decoding size";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  std::vector<BYTE> decodedData(requiredSize);
  if (!CryptStringToBinary(data.c_str(),
                           0,
                           CRYPT_STRING_BASE64,
                           decodedData.data(),
                           &requiredSize,
                           NULL,
                           NULL)) {
    std::string errorMessage = "Error decoding from base64";
    WriteLog(LL_ERROR, errorMessage);
    throw std::runtime_error(errorMessage);
  }
  return decodedData;
}

std::string encryptString(const std::string& text, DWORD flags) {
  /*
   * Based on the API documentation for DPAPI CryptProtectData function
   * https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
   * Adds base64 encoding to the output to avoid returning non printing
   * characters or other tricky stuff in the response.
   */
  if (text.empty()) {
    return "";
  }

  DATA_BLOB dataIn;
  DATA_BLOB dataOut = {0};
  BYTE* pbDataInput = (BYTE*)text.data();
  DWORD cbDataInput = text.length();

  dataIn.pbData = pbDataInput;
  dataIn.cbData = cbDataInput;

  if (!CryptProtectData(&dataIn, NULL, NULL, NULL, NULL, flags, &dataOut)) {
    throw std::runtime_error("Encryption Failed");
  }
  std::vector<BYTE> encryptedData(dataOut.pbData,
                                  dataOut.pbData + dataOut.cbData);
  LocalFree(dataOut.pbData);

  return toBase64(encryptedData);
}

std::string decryptString(const std::string& text, DWORD flags) {
  /*
   * Based on the API documentation for DPAPI CryptUnprotectData function
   * https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
   * Adds base64 decoding to the input to match the encryptString
   * implementation and avoid handling non printing characters that come from
   * raw binary outputs.
   */
  if (text.empty()) {
    return "";
  }

  std::vector<BYTE> encryptedData = fromBase64(text);

  DATA_BLOB dataIn;
  DATA_BLOB dataOut = {0};

  dataIn.pbData = encryptedData.data();
  dataIn.cbData = static_cast<DWORD>(encryptedData.size());

  if (!CryptUnprotectData(&dataIn, NULL, NULL, NULL, NULL, 0, &dataOut)) {
    throw std::runtime_error("Decryption Failed");
  }
  std::string decryptedString(reinterpret_cast<char*>(dataOut.pbData),
                              dataOut.cbData);
  LocalFree(dataOut.pbData);

  return decryptedString;
}

std::string userEncryptString(const std::string& text) {
  // Encrypt a string using the user's login data.
  return encryptString(text, 0);
}

std::string systemEncryptString(const std::string& text) {
  // Encrypt a string using the current computer as context.
  return encryptString(text, CRYPTPROTECT_LOCAL_MACHINE);
}

std::string userDecryptString(const std::string& text) {
  // Decrypt a string using the user's login data.
  return decryptString(text, 0);
}

std::string systemDecryptString(const std::string& text) {
  // Decrypt a string using the user's login data.
  return decryptString(text, 0);
}
