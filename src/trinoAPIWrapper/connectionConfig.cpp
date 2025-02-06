#include "connectionConfig.hpp"
#include <nlohmann/json.hpp>

#include "../util/callbackHelper.hpp"
#include "../util/stringTrim.hpp"
#include "../util/writeLog.hpp"
#include "authProvider/clientCredAuthProvider.hpp"
#include "authProvider/externalAuthProvider.hpp"
#include "authProvider/noAuthProvider.hpp"

using json = nlohmann::json;

static size_t
curlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
  size_t totalSize = size * nmemb;
  s->append(static_cast<char*>(contents), totalSize);
  return totalSize;
}

static size_t
curlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
  std::map<std::string, std::string>* responseHeaderData =
      (std::map<std::string, std::string>*)userdata;
  std::string headerData = std::string(buffer, nitems);
  if (headerData.starts_with("HTTP/")) {
    // The HTTP/<version> header is not a key value pair, so
    // it gets some special logic.
    responseHeaderData->insert({"http", headerData});
  } else {
    auto firstColon = headerData.find(':');
    if (firstColon != std::string::npos) {
      std::string key   = headerData.substr(0, firstColon);
      std::string value = headerData.substr(firstColon + 1);
      // The values usually end in \r\n at a minimum, so we need
      // to trim that off.
      trim(value);
      responseHeaderData->insert({key, value});
    }
  }
  // Return the number of bytes consumed to signal success.
  return nitems * size;
}

ConnectionConfig::ConnectionConfig(std::string hostname,
                                   unsigned short port,
                                   ApiAuthMethod authMethod,
                                   std::string connectionName,
                                   std::string oidcDiscoveryUrl,
                                   std::string clientId,
                                   std::string clientSecret,
                                   std::string oidcScope) {
  this->hostname       = hostname;
  this->port           = port;
  this->connectionName = connectionName;
  this->authMethod     = authMethod;
  this->curl           = nullptr;

  switch (authMethod) {
    case AM_NO_AUTH: {
      this->authConfigPtr = getNoAuthConfigPtr(hostname, port, connectionName);
      break;
    }
    case AM_EXTERNAL_AUTH: {
      this->authConfigPtr =
          getExternalAuthConfigPtr(hostname, port, connectionName);
      break;
    }
    case AM_CLIENT_CRED_AUTH: {
      this->authConfigPtr = getClientCredAuthProvider(hostname,
                                                      port,
                                                      connectionName,
                                                      oidcDiscoveryUrl,
                                                      clientId,
                                                      clientSecret,
                                                      oidcScope);
      break;
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: connection config got unimplemented external auth "
               "method value: " +
                   std::to_string(authMethod));
    }
  }
}

ConnectionConfig::~ConnectionConfig() {
  curl_easy_cleanup(this->curl);
}

std::string const ConnectionConfig::getHostname() {
  return this->hostname;
}

unsigned short const ConnectionConfig::getPort() {
  return this->port;
}

ApiAuthMethod const ConnectionConfig::getAuthMethod() {
  return this->authMethod;
}

std::string const ConnectionConfig::getStatementUrl() {
  return (this->hostname) + ":" + std::to_string(port) + "/v1/statement";
}

CURL* ConnectionConfig::getCurl() {
  /*
  Return a curl handle, reset it if needed, and it's ready to go.
  */
  if (this->curl == nullptr) {
    this->curl = curl_easy_init();
    // We always want to use SSL.
    curl_easy_setopt(this->curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
    // We want to save the response body in a string using a callback.
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &(this->responseData));
    // We want to parse response headers.
    curl_easy_setopt(this->curl, CURLOPT_HEADERFUNCTION, curlHeaderCallback);
    curl_easy_setopt(
        this->curl, CURLOPT_HEADERDATA, &(this->responseHeaderData));
    // Set a timeout on all requests
    curl_easy_setopt(this->curl, CURLOPT_TIMEOUT_MS, 10000);
    // Enable gzip and/or deflate on responses
    curl_easy_setopt(this->curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
  }

curlSetup:
  // Clear the previous response data, we do not want to append to it.
  this->responseData.clear();
  // Clear the previous response headers as well
  this->responseHeaderData.clear();

  // We could do a full reset here, but that seems to slow the driver
  // down considerably. Better to just reset a few things and
  // otherwise reuse the curl handle.
  // curl_easy_reset(this->curl);

  // Let's say the standard state of curl is that the
  // handle is configured to run GET requests, no matter
  // how it was used before.
  curl_easy_setopt(this->curl, CURLOPT_HTTPGET, true);

  // Set up any required headers if needed.
  if (this->authConfigPtr->headers.size() > 0) {
    struct curl_slist* headers = nullptr;
    for (const auto pair : this->authConfigPtr->headers) {
      std::string nextHeader = pair.first + ": " + pair.second;
      headers                = curl_slist_append(headers, nextHeader.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  // Now that we have a fully configured CURL handle, check if we need to do
  // any required auth steps. We may need to use the configured handle to
  // perform the authentication.
  if (this->authConfigPtr->isExpired()) {
    WriteLog(LL_TRACE,
             "  Detected expired authentication. Reauthenticating...");
    this->authConfigPtr->refresh(
        this->curl, &(this->responseData), &(this->responseHeaderData));
    goto curlSetup;
  }

  return this->curl;
}

long ConnectionConfig::getLastHTTPStatusCode() {
  long httpStatusCode = -1;
  if (this->curl) {
    curl_easy_getinfo(this->curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);
  }
  return httpStatusCode;
}

void ConnectionConfig::disconnect() {
  for (std::function f : this->onDisconnectCallbacks) {
    f(this);
  }
  if (this->curl) {
    curl_easy_cleanup(this->curl);
    this->curl = nullptr;
  }
}

std::string ConnectionConfig::getTrinoServerVersion() {
  CURL* curl = this->getCurl();

  std::string url =
      this->hostname + ":" + std::to_string(this->port) + "/v1/info";

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    WriteLog(LL_ERROR,
             "Failed to read trino server version: " +
                 std::string(curl_easy_strerror(res)));
    return "";
  }

  json jsonResponse = nlohmann::json::parse(this->responseData);
  return jsonResponse["nodeVersion"]["version"].get<std::string>();
}

void ConnectionConfig::registerDisconnectCallback(
    std::function<void(ConnectionConfig*)> f) {
  this->onDisconnectCallbacks.push_back(f);
}

void ConnectionConfig::unregisterDisconnectCallback(
    std::function<void(ConnectionConfig*)> f) {
  removeCallbackFromVector(this->onDisconnectCallbacks, f);
}
