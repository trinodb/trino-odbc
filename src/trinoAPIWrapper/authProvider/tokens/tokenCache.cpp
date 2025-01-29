#include "tokenCache.hpp"

#include "../../../util/windowsLean.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <shlobj.h> // For getting windows folder paths.
#include <string>

#include "../../../util/b64decoder.hpp"
#include "../../../util/cryptUtils.hpp"
#include "../../../util/stringSplitAndTrim.hpp"
#include "../../../util/timeUtils.hpp"
#include "../../../util/writeLog.hpp"
#include "tokenParser.hpp"


using json = nlohmann::json;


int TOKEN_CACHE_JSON_INDENT     = 2;
long long EXPIRY_GRACE_PERIOD_S = 60 * 10;


TokenCacheEntry::TokenCacheEntry(std::string accessToken,
                                 std::string refreshToken,
                                 std::string tokenId) {
  this->accessToken  = accessToken;
  this->refreshToken = refreshToken;
  this->tokenId      = tokenId;
  if (not this->accessToken.empty()) {
    this->parsedAccessToken = parseAccessToken(this->accessToken);
  } else {
    // Initialize this to an empty object rather than a
    // the default, which is a null object. This allows
    // other code to use json.value("key", <default>) on
    // the parsed token.
    this->parsedAccessToken = json(json::value_t::object);
  }
}


bool TokenCacheEntry::isExpired() {
  long long currentTimestamp = getSecondsSinceEpoch();
  // We need to get the "exp" key from the parsed access token, but only
  // if it exists. That's harder to do that one would hope.
  long long expiresAt    = this->parsedAccessToken.value<long long>("exp", 0LL);
  long long timeToExpiry = expiresAt - currentTimestamp;
  if (getLogLevel() <= LL_TRACE) {
    WriteLog(LL_TRACE,
             "  Current timestamp: " + std::to_string(currentTimestamp));
    WriteLog(LL_TRACE,
             "  Token expires timestamp: " + std::to_string(expiresAt));
    WriteLog(LL_TRACE,
             "  Token expires in " + std::to_string(timeToExpiry) + " seconds");
  }
  return (timeToExpiry - EXPIRY_GRACE_PERIOD_S) < 0;
}


void TokenCacheEntry::setAccessToken(std::string accessToken) {
  this->accessToken       = accessToken;
  this->parsedAccessToken = parseAccessToken(accessToken);
}

void TokenCacheEntry::setRefreshToken(std::string refreshToken) {
  this->refreshToken = refreshToken;
}

json TokenCacheEntry::getParsedAccessToken() {
  return this->parsedAccessToken;
}

std::string TokenCacheEntry::getAccessToken() {
  return this->accessToken;
}


std::string TokenCacheEntry::getRefreshToken() {
  return this->refreshToken;
}


std::string TokenCacheEntry::getTokenId() {
  return this->tokenId;
}

std::wstring getTempFilePath() {
  PWSTR tempPath = nullptr;
  // https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
  if (SUCCEEDED(
          SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &tempPath))) {
    std::wstring tempPathStr(tempPath);
    // We are responsible for freeing the memory used to return the tempPath.
    CoTaskMemFree(tempPath);
    return tempPathStr + L"\\Temp\\TrinoODBCTokenCache.json";
  }
  return L"";
}


TokenCacheEntry readTokenCache(const std::string& tokenId) {
  std::wstring filePath = getTempFilePath();

  WriteLog(LL_TRACE, std::wstring(L"  Token cache file path is: " + filePath));

  // Handle the case that the file doesn't exist.
  if (!std::filesystem::exists(filePath)) {
    WriteLog(LL_WARN,
             "  WARNING: Token cache file does not exist. Is this the first "
             "run of the driver?");
    return TokenCacheEntry("", "", tokenId);
  }

  // Handle the case that the file can't be parsed as JSON.
  std::ifstream inputFile(filePath);
  json jsonData;
  try {
    inputFile >> jsonData;
  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: failed to parse token cache file as JSON: " +
                 std::string(e.what()));
    inputFile.close();
    return TokenCacheEntry("", "", tokenId);
  }

  // Close the file, at this point we have parsed it to JSON.
  inputFile.close();

  // Get the tokens, if they are available. Be sure to handle the case that
  // we're trying to read a tokenId that's not present in the file. We want to
  // default to empty strings for the tokens in that case.
  json tokenData = jsonData.value(tokenId, json(json::value_t::object));
  std::string encryptedAccessToken =
      tokenData.value("encryptedAccessToken", "");
  std::string encryptedRefreshToken =
      tokenData.value("encryptedRefreshToken", "");
  std::string accessToken  = decryptString(encryptedAccessToken);
  std::string refreshToken = decryptString(encryptedRefreshToken);

  // Return a cache entry for these.
  WriteLog(LL_TRACE, "  Token cache read successfully");
  return TokenCacheEntry(accessToken, refreshToken, tokenId);
}


void writeTokenCache(TokenCacheEntry cacheEntry) {
  std::wstring filePath = getTempFilePath();

  // Start by reading the cache file in its current state.
  std::ifstream inputFile(filePath);
  json inputJsonData;
  if (!inputFile.is_open()) {
    WriteLog(LL_WARN,
             "  ERROR: cannot open token cache file for reading. First time "
             "writing to token cache?");
  } else {
    WriteLog(LL_TRACE,
             "  Reading from token cache before overwriting it fresh");
    inputFile >> inputJsonData;
    inputFile.close();
  }

  // Modify the input with the new token(s) we wish to write.
  std::string tokenId    = cacheEntry.getTokenId();
  inputJsonData[tokenId] = json::object();
  inputJsonData[tokenId]["encryptedAccessToken"] =
      encryptString(cacheEntry.getAccessToken());
  inputJsonData[tokenId]["encryptedRefreshToken"] =
      encryptString(cacheEntry.getRefreshToken());

  // Last, modify it and write it back with an update token cache.
  std::ofstream outputFile(filePath);
  if (!outputFile.is_open()) {
    WriteLog(LL_ERROR, "  ERROR: cannot open token cache file for writing");
  }
  outputFile << inputJsonData.dump(TOKEN_CACHE_JSON_INDENT);
  outputFile.close();
}


std::string getTokenIdentity(std::string hostname,
                             unsigned short port,
                             std::string connectionName) {
  return connectionName + "__" + hostname + "__" + std::to_string(port);
}
