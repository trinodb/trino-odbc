#pragma once

#include <map>
#include <string>

#include "curl/curl.h"

#include "authConfig.hpp"
#include "tokens/tokenCache.hpp"


class TokenCacheAuthProviderBase : public AuthConfig {

  public:
    TokenCacheAuthProviderBase(std::string hostname,
                               unsigned short port,
                               std::string connectionName);
    bool const isExpired() override;
    void applyToken();
    void
    refresh(CURL* curl,
            std::string* responseData,
            std::map<std::string, std::string>* responseHeaderData) override;
    virtual ~TokenCacheAuthProviderBase() = default;

  protected:
    std::string tokenId;
    std::optional<TokenCacheEntry> tokenCache;
    // This is pure virtual, it requires an implementation in the subclass.
    virtual std::string obtainAccessToken(
        CURL* curl,
        std::string* responseData,
        std::map<std::string, std::string>* responseHeaderData) = 0;
    void applyTokenIfNotExpired();
    void cacheToken(std::string accessToken);
    void setAccessTokenHeader(std::string accessToken);
};
