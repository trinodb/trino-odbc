#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class TokenCacheEntry {
  private:
    json parsedAccessToken;
    std::string accessToken;
    std::string refreshToken;
    std::string tokenId;

  public:
    TokenCacheEntry(std::string accessToken,
                    std::string refreshToken,
                    std::string tokenId);
    bool isExpired();
    void setAccessToken(std::string accessToken);
    void setRefreshToken(std::string refreshToken);
    json getParsedAccessToken();
    std::string getAccessToken();
    std::string getRefreshToken();
    std::string getTokenId();
};


TokenCacheEntry readTokenCache(const std::string& tokenid);

std::string getTokenIdentity(std::string hostname,
                             unsigned short port,
                             std::string connectionName);

void writeTokenCache(TokenCacheEntry cacheEntry);
