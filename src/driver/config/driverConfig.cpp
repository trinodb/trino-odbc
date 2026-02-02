#include "driverConfig.hpp"
#include "../../util/capitalize.hpp"
#include "../../util/writeLog.hpp"




std::vector<std::string> LOG_LEVEL_NAMES = {LOG_LEVEL_NONE,
                                            LOG_LEVEL_ERROR,
                                            LOG_LEVEL_WARN,
                                            LOG_LEVEL_INFO,
                                            LOG_LEVEL_DEBUG,
                                            LOG_LEVEL_TRACE};

std::vector<LogLevel> LOG_LEVEL_VALUES = {
    LL_NONE, LL_ERROR, LL_WARN, LL_INFO, LL_DEBUG, LL_TRACE};

std::map<LogLevel, std::string> LOG_LEVEL_TO_LOG_NAME = {
    std::make_pair(LL_TRACE, LOG_LEVEL_TRACE),
    std::make_pair(LL_DEBUG, LOG_LEVEL_DEBUG),
    std::make_pair(LL_INFO, LOG_LEVEL_INFO),
    std::make_pair(LL_WARN, LOG_LEVEL_WARN),
    std::make_pair(LL_ERROR, LOG_LEVEL_ERROR),
    std::make_pair(LL_NONE, LOG_LEVEL_NONE),
};

std::map<std::string, LogLevel> LOG_NAME_TO_LOG_LEVEL = {
    std::make_pair(LOG_LEVEL_TRACE, LL_TRACE),
    std::make_pair(LOG_LEVEL_DEBUG, LL_DEBUG),
    std::make_pair(LOG_LEVEL_INFO, LL_INFO),
    std::make_pair(LOG_LEVEL_WARN, LL_WARN),
    std::make_pair(LOG_LEVEL_ERROR, LL_ERROR),
    std::make_pair(LOG_LEVEL_NONE, LL_NONE),
};

std::vector<std::string> AUTH_METHOD_NAMES = {AUTH_NAME_NO_AUTH,
                                              AUTH_NAME_EXTERNAL_AUTH,
                                              AUTH_NAME_CLIENT_CRED_AUTH,
                                              AUTH_NAME_DEVICE_CODE,
                                              AUTH_NAME_USERNAME_AND_PASSWORD};

std::vector<std::string> OIDC_METHOD_NAMES = {OIDC_ENDPOINT_METHOD_NAME_URL,
OIDC_ENDPOINT_METHOD_NAME_USERINPUT};

std::vector<ApiAuthMethod> AUTH_METHOD_VALUES = {
    AM_NO_AUTH, AM_EXTERNAL_AUTH, AM_CLIENT_CRED_AUTH, AM_DEVICE_CODE, AM_USERNAME_AND_PASSWORD};

std::map<ApiAuthMethod, std::string> AUTH_METHOD_TO_AUTH_NAME = {
    std::make_pair(AM_NO_AUTH, AUTH_NAME_NO_AUTH),
    std::make_pair(AM_EXTERNAL_AUTH, AUTH_NAME_EXTERNAL_AUTH),
    std::make_pair(AM_CLIENT_CRED_AUTH, AUTH_NAME_CLIENT_CRED_AUTH),
    std::make_pair(AM_DEVICE_CODE, AUTH_NAME_DEVICE_CODE),
    std::make_pair(AM_USERNAME_AND_PASSWORD, AUTH_NAME_USERNAME_AND_PASSWORD)};

std::map<std::string, ApiAuthMethod> AUTH_NAME_TO_AUTH_METHOD = {
    std::make_pair(AUTH_NAME_NO_AUTH, AM_NO_AUTH),
    std::make_pair(AUTH_NAME_EXTERNAL_AUTH, AM_EXTERNAL_AUTH),
    std::make_pair(AUTH_NAME_CLIENT_CRED_AUTH, AM_CLIENT_CRED_AUTH),
    std::make_pair(AUTH_NAME_DEVICE_CODE, AM_DEVICE_CODE),
    std::make_pair(AUTH_NAME_USERNAME_AND_PASSWORD, AM_USERNAME_AND_PASSWORD)};

std::map<OidcEndpointMethod, std::string> OIDC_ENDPOINT_METHOD_TO_OIDC_ENDPOINT_NAME = {
    std::make_pair(OEM_URL, OIDC_ENDPOINT_METHOD_NAME_URL),
    std::make_pair(OEM_USER_INPUT, OIDC_ENDPOINT_METHOD_NAME_USERINPUT) };

std::map<std::string, OidcEndpointMethod> OIDC_ENDPOINT_NAME_TO_OIDC_ENDPOINT_METHOD = {
        std::make_pair(OIDC_ENDPOINT_METHOD_NAME_URL, OEM_URL),
        std::make_pair(OIDC_ENDPOINT_METHOD_NAME_USERINPUT, OEM_USER_INPUT)};


// All default values - do not miss any!
std::map<std::string, std::string> DRIVER_CONFIG_DEFAULT_VALUES = {
    std::make_pair(CONFIG_FIELD_HOSTNAME, "localhost"),
    std::make_pair(CONFIG_FIELD_PORT, "8080"),
    std::make_pair(CONFIG_FIELD_LOGLEVEL, "None"),
    std::make_pair(CONFIG_FIELD_AUTHMETHOD, "No Auth"),
    std::make_pair(CONFIG_FIELD_OIDCDISCOVERYURL, ""),
    std::make_pair(CONFIG_FIELD_CLIENTID, ""),
    std::make_pair(CONFIG_FIELD_CLIENTSECRET, ""),
    std::make_pair(CONFIG_FIELD_OIDCSCOPE, ""),
    std::make_pair(CONFIG_FIELD_SECRETENCRYPTIONLEVEL, "user"),
    std::make_pair(CONFIG_FIELD_AUTHENDPOINT, ""),
    std::make_pair(CONFIG_FIELD_USERNAME, ""),
    std::make_pair(CONFIG_FIELD_PASSWORD, "******"),
    std::make_pair(CONFIG_FIELD_OIDCDISCOVERYMETHOD,
                   OIDC_ENDPOINT_METHOD_NAME_URL),
};

// DSN
std::string DriverConfig::getDSN() {
  return this->dsn;
}
void DriverConfig::setDSN(std::string dsn) {
  this->dsn = dsn;
}

// Driver
std::string DriverConfig::getDriver() {
  return this->driver;
}
void DriverConfig::setDriver(std::string driver) {
  this->driver = driver;
}

// Hostname
std::string DriverConfig::getHostname() {
  return this->hostname;
}
void DriverConfig::setHostname(std::string hostname) {
  this->hostname = hostname;
}

// Port - Accepts and returns both integers and strings.
std::string DriverConfig::getPortStr() {
  return std::to_string(this->port);
}
uint16_t DriverConfig::getPortNum() {
  return this->port;
}
void DriverConfig::setPort(std::string port) {
  this->port = std::stoi(port);
}
void DriverConfig::setPort(uint16_t port) {
  this->port = port;
}

// Log Level
std::string DriverConfig::getLogLevelStr() {
  return LOG_LEVEL_TO_LOG_NAME.at(this->logLevel);
}
LogLevel DriverConfig::getLogLevelEnum() {
  return this->logLevel;
}
void DriverConfig::setLogLevel(LogLevel level) {
  this->logLevel = level;
}
void DriverConfig::setLogLevel(std::string level) {
  std::string casedLevel = capitalizedCase(level);
  this->logLevel         = LOG_NAME_TO_LOG_LEVEL.at(casedLevel);
}

// Auth Method
std::string DriverConfig::getAuthMethodStr() {
  return AUTH_METHOD_TO_AUTH_NAME.at(this->authMethod);
}
ApiAuthMethod DriverConfig::getAuthMethodEnum() {
  return this->authMethod;
}
void DriverConfig::setAuthMethod(ApiAuthMethod authMethod) {
  this->authMethod = authMethod;
}
void DriverConfig::setAuthMethod(std::string authMethod) {
  std::string casedAuthMethod = capitalizedCase(authMethod);
  this->authMethod            = AUTH_NAME_TO_AUTH_METHOD.at(casedAuthMethod);
}

// OIDC Discovery URL
std::string DriverConfig::getOidcDiscoveryUrl() {
  return this->oidcDiscoveryUrl;
}
void DriverConfig::setOidcDiscoveryUrl(std::string oidcDiscoveryUrl) {
  this->oidcDiscoveryUrl = oidcDiscoveryUrl;
}

// Client ID
std::string DriverConfig::getClientId() {
  return this->clientId;
}
void DriverConfig::setClientId(std::string clientId) {
  this->clientId = clientId;
}

// Client Secret
std::string DriverConfig::getClientSecret() {
  return this->clientSecret;
}
void DriverConfig::setClientSecret(std::string clientSecret) {
  this->clientSecret = clientSecret;
}

// OIDC Scope
std::string DriverConfig::getOidcScope() {
  return this->oidcScope;
}
void DriverConfig::setOidcScope(std::string oidcScope) {
  this->oidcScope = oidcScope;
}

std::string DriverConfig::getTokenEndpoint() {
  return this->tokenEndpoint;
}
void DriverConfig::setTokenEndpoint(std::string tokenEndpoint) {
  this->tokenEndpoint = tokenEndpoint;
}

std::string DriverConfig::getGrantType() {
  return this->grantType;
}
void DriverConfig::setGrantType(std::string grantType) {
  this->grantType = grantType;
}

// IsSaved
bool DriverConfig::getIsSaved() {
  return this->isSaved;
}
void DriverConfig::setIsSaved(bool isSaved) {
  this->isSaved = isSaved;
}

//Auth Endpoint
std::string DriverConfig::getAuthEndpoint() {
  return this->authEndpoint;
}
void DriverConfig::setAuthEndpoint(const std::string& authEndpoint) {
  this->authEndpoint = authEndpoint;
}

//Username
std::string DriverConfig::getUsername() {
  return this->userName;
}
void DriverConfig::setUsername(const std::string& userName) {
  this->userName = userName;
}

//Password
std::string DriverConfig::getPassword() {
  return this->password;
}
void DriverConfig::setPassword(const std::string& password) {
  this->password = password;
}

//OIDC Endpoint method
std::string DriverConfig::getOidcEndpointMethodStr() {
  return OIDC_ENDPOINT_METHOD_TO_OIDC_ENDPOINT_NAME.at(this->oidcMethod);
}

OidcEndpointMethod DriverConfig::getOidcEndpointMethodEnum() {
  return this->oidcMethod;
}

void DriverConfig::setOidcEndpointMethod(
  const std::string& oidcEndpointMethod) {
  std::string casedOidcEndpointMethod = capitalizedCase(oidcEndpointMethod);
  this->oidcScope            = OIDC_ENDPOINT_NAME_TO_OIDC_ENDPOINT_METHOD.at(casedOidcEndpointMethod);


}
void DriverConfig::setOidcEndpointMethod(
  OidcEndpointMethod oidcEndpointMethod) {
  this->oidcMethod = oidcMethod;
}

// Create a config from key value pairs.
DriverConfig driverConfigFromKVPs(std::map<std::string, std::string> kvps) {
  // KVPs coming from a DSN entry will use (mostly) camelCase.
  // KVPs coming from a connection string will use lowercase.
  // We need to support both, and we need lowercase names from a
  // connection string to override values from the DSN entry if
  // they are present. The override can happen based on the order
  // of differently-cased-but-like-keyed entries in this set of if blocks.
  DriverConfig config;

  if (kvps.count(CONFIG_FIELD_DSN)) {
    config.setDSN(kvps.at(CONFIG_FIELD_DSN));
  }
  if (kvps.count(CONFIG_FIELD_DRIVER)) {
    config.setDriver(kvps.at(CONFIG_FIELD_DRIVER));
  }
  if (kvps.count(CONFIG_FIELD_HOSTNAME)) {
    config.setHostname(kvps.at(CONFIG_FIELD_HOSTNAME));
  }
  if (kvps.count(CONFIG_FIELD_PORT)) {
    config.setPort(kvps.at(CONFIG_FIELD_PORT));
  }
  if (kvps.count(CONFIG_FIELD_LOGLEVEL)) {
    config.setLogLevel(kvps.at(CONFIG_FIELD_LOGLEVEL));
  }
  if (kvps.count(CONFIG_FIELD_AUTHMETHOD)) {
    config.setAuthMethod(kvps.at(CONFIG_FIELD_AUTHMETHOD));
  }
  if (kvps.count(CONFIG_FIELD_OIDCDISCOVERYURL)) {
    config.setOidcDiscoveryUrl(kvps.at(CONFIG_FIELD_OIDCDISCOVERYURL));
  }
  if (kvps.count(CONFIG_FIELD_OIDCDISCOVERYURL_LOWER)) {
    config.setOidcDiscoveryUrl(kvps.at(CONFIG_FIELD_OIDCDISCOVERYURL_LOWER));
  }
  if (kvps.count(CONFIG_FIELD_CLIENTID)) {
    config.setClientId(kvps.at(CONFIG_FIELD_CLIENTID));
  }
  if (kvps.count(CONFIG_FIELD_CLIENTID_LOWER)) {
    config.setClientId(kvps.at(CONFIG_FIELD_CLIENTID_LOWER));
  }
  if (kvps.count(CONFIG_FIELD_CLIENTSECRET)) {
    config.setClientSecret(kvps.at(CONFIG_FIELD_CLIENTSECRET));
  }
  if (kvps.count(CONFIG_FIELD_CLIENTSECRET_LOWER)) {
    config.setClientSecret(kvps.at(CONFIG_FIELD_CLIENTSECRET_LOWER));
  }
  if (kvps.count(CONFIG_FIELD_OIDCSCOPE)) {
    config.setOidcScope(kvps.at(CONFIG_FIELD_OIDCSCOPE));
  }
  if (kvps.count(CONFIG_FIELD_OIDCSCOPE_LOWER)) {
    config.setOidcScope(kvps.at(CONFIG_FIELD_OIDCSCOPE_LOWER));
  }
  if (kvps.count(CONFIG_FIELD_GRANTTYPE)) {
    config.setGrantType(kvps.at(CONFIG_FIELD_GRANTTYPE));
  }
  if (kvps.count(CONFIG_FIELD_TOKENENDPOINT)) {
    config.setTokenEndpoint(kvps.at(CONFIG_FIELD_TOKENENDPOINT));
  }
  if (kvps.count(CONFIG_FIELD_AUTHENDPOINT)) {
    config.setAuthEndpoint(kvps.at(CONFIG_FIELD_AUTHENDPOINT));
  }
  if (kvps.count(CONFIG_FIELD_USERNAME)) {
    config.setUsername(kvps.at(CONFIG_FIELD_USERNAME));
  }
  if (kvps.count(CONFIG_FIELD_PASSWORD)) {
    config.setPassword(kvps.at(CONFIG_FIELD_PASSWORD));
  }
  if (kvps.count(CONFIG_FIELD_OIDCDISCOVERYMETHOD)) {
    config.setOidcEndpointMethod(kvps.at(CONFIG_FIELD_OIDCDISCOVERYMETHOD));
  }
  return config;
}

// Create key value pairs from a config.
std::map<std::string, std::string> driverConfigToKVPs(DriverConfig config) {
  std::map<std::string, std::string> kvps;

  if (!config.getDSN().empty()) {
    kvps[CONFIG_FIELD_DSN] = config.getDSN();
  }
  if (!config.getDriver().empty()) {
    kvps[CONFIG_FIELD_DRIVER] = config.getDriver();
  }
  if (!config.getHostname().empty()) {
    kvps[CONFIG_FIELD_HOSTNAME] = config.getHostname();
  }
  if (!config.getPortStr().empty()) {
    kvps[CONFIG_FIELD_PORT] = config.getPortStr();
  }
  if (!config.getLogLevelStr().empty()) {
    kvps[CONFIG_FIELD_LOGLEVEL] = config.getLogLevelStr();
  }
  if (!config.getAuthMethodStr().empty()) {
    kvps[CONFIG_FIELD_AUTHMETHOD] = config.getAuthMethodStr();
  }
  if (!config.getOidcDiscoveryUrl().empty()) {
    kvps[CONFIG_FIELD_OIDCDISCOVERYURL] = config.getOidcDiscoveryUrl();
  }
  if (!config.getClientId().empty()) {
    kvps[CONFIG_FIELD_CLIENTID] = config.getClientId();
  }
  if (!config.getClientSecret().empty()) {
    kvps[CONFIG_FIELD_CLIENTSECRET] = config.getClientSecret();
  }
  if (!config.getOidcScope().empty()) {
    kvps[CONFIG_FIELD_OIDCSCOPE] = config.getOidcScope();
  }
  if (!config.getAuthEndpoint().empty()) {
    kvps[CONFIG_FIELD_AUTHENDPOINT] = config.getAuthEndpoint();
  }
  if (!config.getUsername().empty()) {
    kvps[CONFIG_FIELD_USERNAME] = config.getUsername();
  }
  if (!config.getPassword().empty()) {
    kvps[CONFIG_FIELD_PASSWORD] = config.getPassword();
  }
  if (!config.getOidcEndpointMethodStr().empty()) {
    kvps[CONFIG_FIELD_OIDCDISCOVERYMETHOD] = config.getOidcEndpointMethodStr();
  }
  if (!config.getTokenEndpoint().empty()) {
    kvps[CONFIG_FIELD_TOKENENDPOINT] = config.getTokenEndpoint();
  }
  return kvps;
}
