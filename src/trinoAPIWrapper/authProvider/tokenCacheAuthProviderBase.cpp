#include "tokenCacheAuthProviderBase.hpp"

TokenCacheAuthProviderBase::TokenCacheAuthProviderBase(
    std::string hostname, unsigned short port, std::string connectionName)
    : AuthConfig(hostname, port, connectionName) {
  this->tokenId = getTokenIdentity(hostname, port, connectionName);
  this->tokenCache =
      std::optional<TokenCacheEntry>(readTokenCache(this->tokenId));
  if (not this->tokenCache->isExpired()) {
    this->applyToken();
  }
}

bool const TokenCacheAuthProviderBase::isExpired() {
  return this->tokenCache->isExpired();
}

void TokenCacheAuthProviderBase::applyToken() {
  std::string accessToken = this->tokenCache->getAccessToken();
  std::string authKey     = "Authorization";
  std::string authValue   = "Bearer " + accessToken;
  this->headers.insert({authKey, authValue});
}

void TokenCacheAuthProviderBase::refresh(
    CURL* curl,
    std::string* responseData,
    std::map<std::string, std::string>* responseHeaderData) {
  applyTokenIfNotExpired();

  // Actually obtain the access token. This function is pure virtual,
  // so subclasses must implement it.
  std::string accessToken =
      this->obtainAccessToken(curl, responseData, responseHeaderData);

  // Cache the token
  this->cacheToken(accessToken);

  // Set the header for all requests
  this->setAccessTokenHeader(accessToken);
}

void TokenCacheAuthProviderBase::applyTokenIfNotExpired() {
  // The first thing to do when asked to refresh the token is to check
  // the token cache to see if we just need to apply it. Not having
  // a token in the request headers yet is an example of an "Expired
  // Token" that requires a "refresh" too.
  if (not this->tokenCache->isExpired()) {
    this->applyToken();
  }
}

void TokenCacheAuthProviderBase::cacheToken(std::string accessToken) {
  // Cache the auth token for later
  this->tokenCache->setAccessToken(accessToken);
  writeTokenCache(this->tokenCache.value());
}

void TokenCacheAuthProviderBase::setAccessTokenHeader(std::string accessToken) {
  // Set the access token into the headers
  std::string authKey   = "Authorization";
  std::string authValue = "Bearer " + accessToken;
  this->headers.insert({authKey, authValue});
}
