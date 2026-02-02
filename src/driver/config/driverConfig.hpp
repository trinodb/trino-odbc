#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "../../trinoAPIWrapper/apiAuthMethod.hpp"
#include "../../trinoAPIWrapper/apiOidcMethod.hpp"
#include "../../util/writeLog.hpp"


extern std::map<std::string, std::string> DRIVER_CONFIG_DEFAULT_VALUES;
extern std::vector<std::string> LOG_LEVEL_NAMES;
extern std::vector<std::string> AUTH_METHOD_NAMES;
extern std::vector<std::string> OIDC_METHOD_NAMES;

constexpr auto CONFIG_FIELD_DSN                       = "dsn";
constexpr auto CONFIG_FIELD_DSN_UPPER                 = "DSN";
constexpr auto CONFIG_FIELD_DRIVER                    = "driver";
constexpr auto CONFIG_FIELD_HOSTNAME                  = "hostname";
constexpr auto CONFIG_FIELD_PORT                      = "port";
constexpr auto CONFIG_FIELD_LOGLEVEL                  = "loglevel";
constexpr auto CONFIG_FIELD_AUTHMETHOD                = "authmethod";
constexpr auto CONFIG_FIELD_OIDCDISCOVERYURL          = "oidcDiscoveryUrl";
constexpr auto CONFIG_FIELD_OIDCDISCOVERYURL_LOWER    = "oidcdiscoveryurl";
constexpr auto CONFIG_FIELD_CLIENTID                  = "clientId";
constexpr auto CONFIG_FIELD_CLIENTID_LOWER            = "clientid";
constexpr auto CONFIG_FIELD_CLIENTSECRET              = "clientSecret";
constexpr auto CONFIG_FIELD_CLIENTSECRET_LOWER        = "clientsecret";
constexpr auto CONFIG_FIELD_OIDCSCOPE                 = "oidcScope";
constexpr auto CONFIG_FIELD_OIDCSCOPE_LOWER           = "oidcscope";
constexpr auto CONFIG_FIELD_GRANTTYPE                 = "granttype";
constexpr auto CONFIG_FIELD_TOKENENDPOINT             = "tokenendpoint";
constexpr auto CONFIG_FIELD_AUTHENDPOINT              = "authenticationendpoint";
constexpr auto CONFIG_FIELD_SECRETENCRYPTIONLEVEL     = "secretEncryptionLevel";
constexpr auto CONFIG_FIELD_USERNAME                  = "userName";
constexpr auto CONFIG_FIELD_PASSWORD                  = "password";
constexpr auto CONFIG_FIELD_OIDCDISCOVERYMETHOD       = "oidcDicoveryMethod";
constexpr auto CONFIG_FIELD_ENCRYPTEDCLIENTSECRET     = "encryptedClientSecret";

// Log Levels

constexpr auto LOG_LEVEL_TRACE      = "Trace";
constexpr auto LOG_LEVEL_ERROR     = "Error";
constexpr auto LOG_LEVEL_WARN       = "Warn";
constexpr auto LOG_LEVEL_INFO       = "Info";
constexpr auto LOG_LEVEL_DEBUG      = "Debug";
constexpr auto LOG_LEVEL_NONE       = "None";

// Auth Methods
constexpr auto AUTH_NAME_NO_AUTH                   = "No Auth";
constexpr auto AUTH_NAME_EXTERNAL_AUTH             = "External Auth";
constexpr auto AUTH_NAME_CLIENT_CRED_AUTH          = "Oidc Client Cred Auth";
constexpr auto AUTH_NAME_DEVICE_CODE               = "Device Code";
constexpr auto AUTH_NAME_USERNAME_AND_PASSWORD     = "Username And Password";


// OIDC Discovery Type
constexpr auto OIDC_ENDPOINT_METHOD_NAME_USERINPUT = "User Input";
constexpr auto OIDC_ENDPOINT_METHOD_NAME_URL       = "Url";


class DriverConfig {
  private:
    // Actual configuration values.
    std::string dsn              = "";
    std::string driver           = "";
    std::string hostname         = "";
    uint16_t port                = 0;
    LogLevel logLevel            = LL_NONE;
    ApiAuthMethod authMethod     = AM_NO_AUTH;
    std::string oidcDiscoveryUrl = "";
    std::string clientId         = "";
    std::string clientSecret     = "";
    std::string oidcScope        = "";
    std::string tokenEndpoint    = "";
    std::string grantType        = "";
    std::string authEndpoint     = "";
    std::string userName         = "";
    std::string password         = "";
    OidcEndpointMethod oidcMethod = OEM_URL;

    // Metadata describing the status of this config object.
    bool isSaved = false;

  public:
    std::string getDSN();
    void setDSN(std::string dsn);

    std::string getDriver();
    void setDriver(std::string driver);

    std::string getHostname();
    void setHostname(std::string hostname);

    std::string getPortStr();
    uint16_t getPortNum();
    void setPort(std::string port);
    void setPort(uint16_t port);

    std::string getLogLevelStr();
    LogLevel getLogLevelEnum();
    void setLogLevel(LogLevel level);
    void setLogLevel(std::string level);

    std::string getAuthMethodStr();
    ApiAuthMethod getAuthMethodEnum();
    void setAuthMethod(std::string authMethod);
    void setAuthMethod(ApiAuthMethod authMethod);

    std::string getOidcDiscoveryUrl();
    void setOidcDiscoveryUrl(std::string oidcDiscoveryUrl);

    std::string getClientId();
    void setClientId(std::string clientId);

    std::string getClientSecret();
    void setClientSecret(std::string clientSecret);

    std::string getOidcScope();
    void setOidcScope(std::string oidcScope);

    bool getIsSaved();
    void setIsSaved(bool isSaved);

    std::string getTokenEndpoint();
    void setTokenEndpoint(std::string tokenEndpoint);

    std::string getGrantType();
    void setGrantType(std::string grantType);

    std::string getAuthEndpoint();
    void setAuthEndpoint(const std::string& authEndpoint);

    
    std::string getUsername();
    void setUsername(const std::string& userName);

    
    std::string getPassword();
    void setPassword(const std::string& password);

    std::string getOidcEndpointMethodStr();
    OidcEndpointMethod getOidcEndpointMethodEnum();
    void setOidcEndpointMethod(const std::string& oidcEndpointMethod);
    void setOidcEndpointMethod(OidcEndpointMethod oidcEndpointMethod);


    std::string serialize();
    static DriverConfig deserialize(const std::string& jsonStr);
};

DriverConfig driverConfigFromKVPs(std::map<std::string, std::string> kvps);
std::map<std::string, std::string> driverConfigToKVPs(DriverConfig config);
