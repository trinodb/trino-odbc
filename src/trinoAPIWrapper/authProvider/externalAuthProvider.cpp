#include "externalAuthProvider.hpp"

#include <map>

#include "nlohmann/json.hpp"

#include "../../util/browserInteraction.hpp"
#include "../../util/delimKvpHelper.hpp"
#include "../../util/writeLog.hpp"

#include "tokenCacheAuthProviderBase.hpp"
#include "tokens/tokenCache.hpp"

using json = nlohmann::json;

/*
Depending on network latency, it might be
necessary to hit the auth token endpoint more than
once.
*/
int MAX_AUTH_TOKEN_RETRIES                 = 2;
std::string EXTERNAL_AUTH_TRIGGER_ENDPOINT = "v1/statement";
std::string EXTERNAL_AUTH_TRIGGER_QUERY    = "SELECT 'authenticating...'";


struct ExternalAuthParams {
    std::string hostname                                   = "";
    unsigned short port                                    = 0;
    CURL* curl                                             = nullptr;
    std::string* responseData                              = nullptr;
    std::map<std::string, std::string>* responseHeaderData = nullptr;
    std::map<std::string, std::string>* requestHeaders     = nullptr;
};


std::string refreshExternalAuth(ExternalAuthParams& params) {
  if (not params.responseData) {
    return "";
  }
  if (not params.responseHeaderData) {
    return "";
  }

  WriteLog(LL_TRACE, "  Attempting external authentication");

  std::string URL = params.hostname + ":" + std::to_string(params.port) + "/" +
                    EXTERNAL_AUTH_TRIGGER_ENDPOINT;
  // In order to make sure we reauthenticate, we need to NOT pass
  // our current auth credentials. This way we will be asked to
  // authenticate again fresh. One way to do that is just to clear out all
  // custom headers from the request.
  // In theory this could be modified to only remove the
  // "Authorization" header, but that's significantly more work.
  struct curl_slist* headers = nullptr;
  curl_easy_setopt(params.curl, CURLOPT_HTTPHEADER, headers);
  curl_slist_free_all(headers);

  // Clear out the buffers for curl callbacks so we
  // don't end up with data from the prior CURL request
  params.responseData->clear();
  params.responseHeaderData->clear();
  curl_easy_setopt(params.curl, CURLOPT_URL, URL.c_str());
  curl_easy_setopt(
      params.curl, CURLOPT_POSTFIELDS, EXTERNAL_AUTH_TRIGGER_QUERY.c_str());

  // Now hit the Trino API, which will return a 401.
  CURLcode res = curl_easy_perform(params.curl);

  // Get the HTTP status code so we can log it in case of an error.
  long http_code = 0;
  curl_easy_getinfo(params.curl, CURLINFO_RESPONSE_CODE, &http_code);

  WriteLog(LL_TRACE,
           "  Auth trigger CURLcode returned: " + std::to_string(res));

  if (not params.responseHeaderData->count("www-authenticate")) {
    WriteLog(LL_ERROR,
             "  ERROR: Unauthenticated request did not return www-authenticate "
             "header");
    WriteLog(LL_ERROR, "  URL was: " + URL);
    WriteLog(LL_ERROR, "  CURLcode was: " + std::to_string(res));
    WriteLog(LL_ERROR, "  HTTP Code was: " + std::to_string(http_code));
    return "";
  }

  // Parse the www-authenticate header to find the URL that we need
  // to navigate to in order to trigger external authentication to work.
  // The header uses this form:
  // www-authenticate: Bearer x_redirect_server="https://...",
  // x_token_server="https://..." This is basically a "Bearer " prefix on a
  // comma delimited set of key-value pairs
  std::string headerKVPs = params.responseHeaderData->at("www-authenticate");

  // We need to strip the word "Bearer" off of the front of this text before
  // we try to parse it. Also take the space character following the word.
  std::string wordBearer = "Bearer";
  headerKVPs.erase(0, wordBearer.size() + 1);

  // The www-authenticate header contains a comma delimited list
  // of key vaule pairs.
  auto authServerInfo = parseKVPsFromCommaDelimStr(headerKVPs);

  // One of the key-value pairs is the x_redirect_server, which is
  // where you go to start authentication. It's an endpoint on the
  // Trino coordinator.
  std::string redirectServer = authServerInfo.at("x_redirect_server");
  WriteLog(LL_INFO, "  Authenticating to: " + redirectServer);

  // This kicks off the auth process in a browser window.
  openURLInDefaultBrowser(redirectServer);

  // Once that's done, we need to hit the a token server to obtain
  // an auth token. This is another endpoint on the trino coordinator
  // The coordinator is kind enough to block this call for a few
  // seconds while the redirect/auth step is handled. That means
  // we don't need to wait here in the driver. We may need to retry
  // though, if the auth between the coordinator and the identity
  // provider is slow.
  int tries               = 0;
  std::string tokenServer = authServerInfo.at("x_token_server");
  while (tries <= MAX_AUTH_TOKEN_RETRIES) {
    // Clear out the buffers for curl callbacks so we
    // don't end up with data from the prior CURL request
    params.responseData->clear();
    params.responseHeaderData->clear();

    // Set the URL to the token server endpoint.
    curl_easy_setopt(params.curl, CURLOPT_URL, tokenServer.c_str());

    // Change from a POST back to a GET request.
    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, nullptr);
    curl_easy_setopt(params.curl, CURLOPT_HTTPGET, 1L);

    // Hit the token server
    CURLcode res = curl_easy_perform(params.curl);

    // Parse the response as JSON, it will contain a "token" key
    // that is the access token.
    json responseJson = json::parse(*params.responseData);
    if (responseJson.contains("token")) {
      // This is the success path. We expect to hit this every time
      // for a DB that is actually set up with token auth.
      WriteLog(LL_INFO, "  Authentication completed successfully");
      return responseJson["token"];
    } else if (responseJson.contains("next_uri")) {
      tokenServer = responseJson["next_uri"];
    }
    tries++;
  }
  // This is the failure path, we didn't get a token.
  WriteLog(LL_ERROR, "  External auth failed");
  return "";
}


class ExternalAuthConfig : public TokenCacheAuthProviderBase {

  public:
    ExternalAuthConfig(std::string hostname,
                       unsigned short port,
                       std::string connectionName)
        : TokenCacheAuthProviderBase(hostname, port, connectionName) {}

    std::string
    obtainAccessToken(CURL* curl,
                      std::string* responseData,
                      std::map<std::string, std::string>* responseHeaderData) {
      ExternalAuthParams params;
      params.curl               = curl;
      params.hostname           = this->hostname;
      params.port               = this->port;
      params.responseData       = responseData;
      params.responseHeaderData = responseHeaderData;
      params.requestHeaders     = &this->headers;

      // Obtain a fresh auth token
      return refreshExternalAuth(params);
    }

    virtual ~ExternalAuthConfig() = default;
};


std::unique_ptr<AuthConfig> getExternalAuthConfigPtr(
    std::string hostname, unsigned short port, std::string connectionName) {
  return std::make_unique<ExternalAuthConfig>(hostname, port, connectionName);
}
