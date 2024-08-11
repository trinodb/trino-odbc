#include "clientCredAuthProvider.hpp"

#include "nlohmann/json.hpp"

#include "../../util/writeLog.hpp"

#include "tokenCacheAuthProviderBase.hpp"
#include "tokens/tokenCache.hpp"

using json = nlohmann::json;

std::string urlEncode(CURL* curl, std::string s) {
  // Wrap curl_easy_escape so we can make sure any memory
  // allocated is immediately freed.
  char* encoded        = curl_easy_escape(curl, s.c_str(), s.length());
  std::string toReturn = std::string(encoded);
  curl_free(encoded);
  return toReturn;
}

struct ClientCredAuthParams {
    std::string hostname                                   = "";
    unsigned short port                                    = 0;
    CURL* curl                                             = nullptr;
    std::string* oidcDiscoveryUrl                          = nullptr;
    std::string* clientId                                  = nullptr;
    std::string* clientSecret                              = nullptr;
    std::string* scope                                     = nullptr;
    std::string* responseData                              = nullptr;
    std::map<std::string, std::string>* responseHeaderData = nullptr;
    std::map<std::string, std::string>* requestHeaders     = nullptr;
};

std::string refreshClientCredAuth(ClientCredAuthParams& params) {
  // Obtain the OIDC Discovery data.
  curl_easy_setopt(params.curl, CURLOPT_URL, params.oidcDiscoveryUrl->c_str());
  CURLcode res1 = curl_easy_perform(params.curl);
  WriteLog(LL_DEBUG,
           "  OIDC discovery CURLcode response was: " + std::to_string(res1));
  json discoveryData = json::parse(*params.responseData);

  // Obtain the token endpoint that provides tokens in exchange for
  // client credentials.
  std::string tokenEndpoint = discoveryData["token_endpoint"];
  WriteLog(LL_TRACE, "  Token Endpoint Was: " + tokenEndpoint);

  // Construct a POST body for the token endpoint.
  // It must use x-www-form-urlencoded encoding.
  std::string grantType           = "client_credentials";
  std::string clientId            = *params.clientId;
  std::string clientSecret        = *params.clientSecret;
  std::string scope               = *params.scope;
  std::string encodedGrantType    = urlEncode(params.curl, grantType);
  std::string encodedClientId     = urlEncode(params.curl, clientId);
  std::string encodedClientSecret = urlEncode(params.curl, clientSecret);
  std::string encodedScope        = urlEncode(params.curl, scope);
  std::ostringstream oss;
  oss << "grant_type=" << encodedGrantType;
  oss << "&client_id=" << encodedClientId;
  oss << "&client_secret=" << encodedClientSecret;
  oss << "&scope=" << encodedScope;
  std::string tokenPost = oss.str();

  // POST the creds and required data to exchange it for a token
  params.responseData->clear();
  params.responseHeaderData->clear();
  curl_easy_setopt(params.curl, CURLOPT_URL, tokenEndpoint.c_str());
  curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, tokenPost.c_str());
  struct curl_slist* headers = nullptr;
  headers                    = curl_slist_append(
      headers, "Content-Type: application/x-www-form-urlencoded");
  curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
  CURLcode res2 = curl_easy_perform(params.curl);
  WriteLog(LL_DEBUG,
           "  Token endpoint HTTP response code was: " + std::to_string(res2));

  // Read the token out of the response
  json responseJson = json::parse(*params.responseData);
  if (responseJson.contains("access_token")) {
    // This is the success path, assuming we get a valid token.
    WriteLog(LL_INFO, "  Client cred auth completed successfully");
    return responseJson["access_token"];
  }
  // This is the failure path, we didn't get a token.
  WriteLog(LL_ERROR, "  Client cred auth failed");
  return "";
}

class ClientCredAuthConfig : public TokenCacheAuthProviderBase {
  private:
    std::string oidcDiscoveryUrl;
    std::string clientId;
    std::string clientSecret;
    std::string scope;

  public:
    ClientCredAuthConfig(std::string hostname,
                         unsigned short port,
                         std::string connectionName,
                         std::string oidcDiscoveryUrl,
                         std::string clientId,
                         std::string clientSecret,
                         std::string scope)
        : TokenCacheAuthProviderBase(hostname, port, connectionName) {
      // The other parameters are used by base classes.
      this->oidcDiscoveryUrl = oidcDiscoveryUrl;
      this->clientId         = clientId;
      this->clientSecret     = clientSecret;
      this->scope            = scope;
    }


    std::string
    obtainAccessToken(CURL* curl,
                      std::string* responseData,
                      std::map<std::string, std::string>* responseHeaderData) {
      ClientCredAuthParams params;
      params.curl               = curl;
      params.hostname           = this->hostname;
      params.port               = this->port;
      params.responseData       = responseData;
      params.responseHeaderData = responseHeaderData;
      params.requestHeaders     = &this->headers;
      params.oidcDiscoveryUrl   = &this->oidcDiscoveryUrl;
      params.clientId           = &this->clientId;
      params.clientSecret       = &this->clientSecret;
      params.scope              = &this->scope;

      return refreshClientCredAuth(params);
    }

    virtual ~ClientCredAuthConfig() = default;
};

std::unique_ptr<AuthConfig>
getClientCredAuthProvider(std::string hostname,
                          unsigned short port,
                          std::string connectionName,
                          std::string oidcDiscoveryUrl,
                          std::string clientId,
                          std::string clientSecret,
                          std::string oidcScope) {
  if (connectionName.empty()) {
    // If there is no connection name, that means this is a connection
    // defined entirely by the connection string. In that case we can
    // substitute the clientId and scope together as the name. This is
    // important because the name is used as a key for the token cache
    // layer and we don't want cache collisions between dsn-less
    // connection strings to the same host/port.
    return std::make_unique<ClientCredAuthConfig>(hostname,
                                                  port,
                                                  clientId + "__" + oidcScope,
                                                  oidcDiscoveryUrl,
                                                  clientId,
                                                  clientSecret,
                                                  oidcScope);
  } else {
    return std::make_unique<ClientCredAuthConfig>(hostname,
                                                  port,
                                                  connectionName,
                                                  oidcDiscoveryUrl,
                                                  clientId,
                                                  clientSecret,
                                                  oidcScope);
  }
}
