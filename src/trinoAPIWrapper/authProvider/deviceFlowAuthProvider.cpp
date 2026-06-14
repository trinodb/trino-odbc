#include <regex>
#include <thread>

#include "clientCredAuthProvider.hpp"

#include "nlohmann/json.hpp"

#include "../../util/writeLog.hpp"

#include "tokenCacheAuthProviderBase.hpp"
#include "tokens/tokenCache.hpp"

using json = nlohmann::json;

std::string url_encode(CURL* curl, std::string s) {
  // Wrap curl_easy_escape so we can make sure any memory
  // allocated is immediately freed.
  char* encoded        = curl_easy_escape(curl, s.c_str(), (int)s.length());
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
    std::string* grantType                                 = nullptr;
    std::string* tokenEndpoint                             = nullptr;
    std::string* responseData                              = nullptr;
    std::map<std::string, std::string>* responseHeaderData = nullptr;
    std::map<std::string, std::string>* requestHeaders     = nullptr;
};

std::string refreshDeviceCredAuth(ClientCredAuthParams& params) {
  // Obtain the OIDC Discovery data.
  curl_easy_setopt(params.curl, CURLOPT_URL, params.oidcDiscoveryUrl->c_str());
  CURLcode res1 = curl_easy_perform(params.curl);
  WriteLog(LL_DEBUG,
           "  OIDC discovery CURLcode response was: " + std::to_string(res1));
  json discoveryData = json::parse(*params.responseData);

  WriteLog(LL_DEBUG,
           "Full discovery response: " +
               discoveryData.dump(4)); // Pretty-print with 4 spaces

  // Obtain the token endpoint that provides tokens in exchange for
  // client credentials.
  std::string deviceEndpoint = discoveryData["device_authorization_endpoint"];
  std::string tokenEndpoint  = discoveryData["token_endpoint"];
  WriteLog(LL_TRACE,
           "  OIDC token Endpoint Was: " + tokenEndpoint +
               ", deviceEndpoint:" + deviceEndpoint);

  // Construct a POST body for the token endpoint.
  // It must use x-www-form-urlencoded encoding.
  std::string clientId            = *params.clientId;
  std::string clientSecret        = *params.clientSecret;
  std::string encodedClientId     = url_encode(params.curl, clientId);
  std::string encodedClientSecret = url_encode(params.curl, clientSecret);
  std::ostringstream oss;
  oss << "&client_id=" << encodedClientId;
  oss << "&client_secret=" << encodedClientSecret;
  std::string tokenPost = oss.str();

  // POST the creds and required data to exchange it for a token
  params.responseData->clear();
  params.responseHeaderData->clear();
  curl_easy_setopt(params.curl, CURLOPT_URL, deviceEndpoint.c_str());
  curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, tokenPost.c_str());
  struct curl_slist* headers = nullptr;
  headers                    = curl_slist_append(
      headers, "Content-Type: application/x-www-form-urlencoded");
  curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
  CURLcode res2 = curl_easy_perform(params.curl);
  curl_slist_free_all(
      headers); // Always free curl headers to avoid memory leaks
  WriteLog(LL_DEBUG,
           "  Token endpoint HTTP response code was: " + std::to_string(res2));

  // Read the token out of the response
  json responseJson = json::parse(*params.responseData);

  WriteLog(LL_DEBUG,
           "Full token response: " +
               responseJson.dump(4)); // Pretty-print with 4 spaces


  if (responseJson.contains("verification_uri_complete")) {
    // Log the verification URI for the user
    std::string verificationUriComplete =
        responseJson["verification_uri_complete"];
    std::string deviceCode = responseJson["device_code"];
    int expiresIn          = responseJson["expires_in"];
    int interval           = responseJson["interval"];

    WriteLog(LL_INFO, "  User must visit: " + verificationUriComplete);
    WriteLog(LL_INFO, "  Device code: " + deviceCode);
    WriteLog(LL_INFO, "  Polling for token...");

    std::string command = "start " + verificationUriComplete;

    int result = std::system(command.c_str());
    if (result != 0) {
      WriteLog(LL_ERROR, "  Failed to open the browser. Command: " + command);
    }

    // Start polling for the token
    auto startTime = std::chrono::steady_clock::now();
    while (true) {
      // Check if the polling has timed out
      auto currentTime = std::chrono::steady_clock::now();
      int elapsedTime  = std::chrono::duration_cast<std::chrono::seconds>(
                            currentTime - startTime)
                            .count();
      if (elapsedTime >= expiresIn) {
        WriteLog(LL_ERROR, "  Device code expired before token was obtained.");
        return "";
      }

      // Prepare the polling request
      std::ostringstream oss;
      oss << "grant_type=urn:ietf:params:oauth:grant-type:device_code";
      oss << "&device_code=" << url_encode(params.curl, deviceCode);
      oss << "&client_id=" << encodedClientId;
      oss << "&client_secret=" << encodedClientSecret;
      std::string tokenPost = oss.str();

      // Clear previous response data
      params.responseData->clear();
      params.responseHeaderData->clear();

      headers = nullptr;
      headers                    = curl_slist_append(
          headers, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
      // Send the polling request
      curl_easy_setopt(params.curl, CURLOPT_URL, tokenEndpoint.c_str());
      curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, tokenPost.c_str());
      CURLcode res = curl_easy_perform(params.curl);
      WriteLog(LL_DEBUG,
               "  Polling token endpoint HTTP response code: " +
                   std::to_string(res));
      curl_slist_free_all(
          headers); // Always free curl headers to avoid memory leaks

      // Parse the response
      json pollingResponse = json::parse(*params.responseData);
      WriteLog(LL_DEBUG, "  Polling response: " + pollingResponse.dump(4));

      // Check if the token is available
      if (pollingResponse.contains("access_token")) {
        WriteLog(LL_INFO, "  Token obtained successfully.");
        return pollingResponse["access_token"];
      }

      // Handle errors
      if (pollingResponse.contains("error")) {
        std::string error = pollingResponse["error"];
        if (error == "authorization_pending") {
          WriteLog(LL_DEBUG, "  Authorization pending, retrying...");
        } else if (error == "slow_down") {
          WriteLog(LL_DEBUG,
                   "  Server requested slower polling, increasing interval...");
          interval += 5; // Adjust interval as per server's suggestion
        } else {
          WriteLog(LL_ERROR, "  Polling failed with error: " + error);
          return "";
        }
      }

      // Wait for the specified interval before polling again
      std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
  }

  // This is the failure path, we didn't get a token.
  WriteLog(LL_ERROR, "  Client cred auth failed");
  return "";
}

class DeviceCredAuthConfig : public TokenCacheAuthProviderBase {
  private:
    std::string oidcDiscoveryUrl;
    std::string clientId;
    std::string clientSecret;
    std::string scope;
    std::string grantType;
    std::string tokenEndpoint;

  public:
    DeviceCredAuthConfig(std::string hostname,
                         unsigned short port,
                         std::string connectionName,
                         std::string oidcDiscoveryUrl,
                         std::string clientId,
                         std::string clientSecret,
                         std::string scope,
                         std::string grantType,
                         std::string tokenEndpoint)
        : TokenCacheAuthProviderBase(hostname, port, connectionName) {
      // The other parameters are used by base classes.
      this->oidcDiscoveryUrl = oidcDiscoveryUrl;
      this->clientId         = clientId;
      this->clientSecret     = clientSecret;
      this->scope            = scope;
      this->grantType        = grantType;
      this->tokenEndpoint    = tokenEndpoint;
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
      params.grantType          = &this->grantType;
      params.tokenEndpoint      = &this->tokenEndpoint;

      return refreshDeviceCredAuth(params);
    }

    virtual ~DeviceCredAuthConfig() = default;
};

std::unique_ptr<AuthConfig>
getDeviceFlowAuthProvider(std::string hostname,
                          unsigned short port,
                          std::string connectionName,
                          std::string oidcDiscoveryUrl,
                          std::string clientId,
                          std::string clientSecret,
                          std::string oidcScope) {
  // If there is no connection name, that means this is a connection
  // defined entirely by the connection string. In that case we can
  // substitute the clientId and scope together as the name. This is
  // important because the name is used as a key for the token cache
  // layer and we don't want cache collisions between dsn-less
  // connection strings to the same host/port.
  return std::make_unique<DeviceCredAuthConfig>(hostname,
                                                port,
                                                clientId + "__" + oidcScope,
                                                oidcDiscoveryUrl,
                                                clientId,
                                                clientSecret,
                                                oidcScope,
                                                "",
                                                "");
}
