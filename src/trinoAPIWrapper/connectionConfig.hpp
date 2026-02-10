#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <curl/curl.h>

#include "apiAuthMethod.hpp"
#include "apiOidcMethod.hpp"
#include "authProvider/authConfig.hpp"
#include "environmentConfig.hpp"

class ConnectionConfig {
  private:
    std::string hostname;
    unsigned short port;
    std::string connectionName;
    std::string tokenEndpoint;
    std::string grantType;
    std::string userName;
    std::string password;

    ApiAuthMethod authMethod;
    OidcEndpointMethod oidcMethod;
    std::unique_ptr<AuthConfig> authConfigPtr;
    std::vector<std::function<void(ConnectionConfig*)>> onDisconnectCallbacks;

    // CURL is managed within the connection config. This way
    // we can set up all the right headers and SSL options
    // every time anything asks for a CURL handle.
    CURL* curl;

  public:
    ConnectionConfig(const std::string& hostname,
                     unsigned short port,
                     ApiAuthMethod authMethod,
                     const std::string& connectionName,
                     const std::string& oidcDiscoveryUrl,
                     const std::string& clientId,
                     const std::string& clientSecret,
                     const std::string& oidcScope,
                     const std::string& grantType,
                     const std::string& tokenEndpoint,
                     const std::string& authEndpoint,
                     const std::string& userName,
                     const std::string& password,
                     OidcEndpointMethod oidcMethod);

    ~ConnectionConfig();
    std::string const getHostname();
    std::string const getStatementUrl();
    unsigned short const getPort();
    ApiAuthMethod const getAuthMethod();
    CURL* getCurl();
    long getLastHTTPStatusCode();
    void disconnect();
    std::string getTrinoServerVersion();
    void registerDisconnectCallback(std::function<void(ConnectionConfig*)> f);
    void unregisterDisconnectCallback(std::function<void(ConnectionConfig*)> f);

    // Making these public because they're frequently accessed and
    // manipulated external to the ConnectionConfig object
    std::string responseData;
    std::map<std::string, std::string> responseHeaderData;
};
