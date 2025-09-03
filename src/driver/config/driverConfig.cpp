#include "driverConfig.hpp"
#include "../../util/capitalize.hpp"
#include "../../util/writeLog.hpp"


std::vector<std::string> LOG_LEVEL_NAMES = {
    "None", "Error", "Warn", "Info", "Debug", "Trace"};

std::vector<LogLevel> LOG_LEVEL_VALUES = {
    LL_NONE, LL_ERROR, LL_WARN, LL_INFO, LL_DEBUG, LL_TRACE};

std::map<LogLevel, std::string> LOG_LEVEL_TO_LOG_NAME = {
    std::make_pair(LL_TRACE, "Trace"),
    std::make_pair(LL_DEBUG, "Debug"),
    std::make_pair(LL_INFO, "Info"),
    std::make_pair(LL_WARN, "Warn"),
    std::make_pair(LL_ERROR, "Error"),
    std::make_pair(LL_NONE, "None"),
};

std::map<std::string, LogLevel> LOG_NAME_TO_LOG_LEVEL = {
    std::make_pair("Trace", LL_TRACE),
    std::make_pair("Debug", LL_DEBUG),
    std::make_pair("Info", LL_INFO),
    std::make_pair("Warn", LL_WARN),
    std::make_pair("Error", LL_ERROR),
    std::make_pair("None", LL_NONE),
};

std::vector<std::string> AUTH_METHOD_NAMES = {
    "No Auth", "External Auth", "OIDC Client Cred Auth"};

std::vector<ApiAuthMethod> AUTH_METHOD_VALUES = {
    AM_NO_AUTH, AM_EXTERNAL_AUTH, AM_CLIENT_CRED_AUTH};

std::map<ApiAuthMethod, std::string> AUTH_METHOD_TO_AUTH_NAME = {
    std::make_pair(AM_NO_AUTH, "No Auth"),
    std::make_pair(AM_EXTERNAL_AUTH, "External Auth"),
    std::make_pair(AM_CLIENT_CRED_AUTH, "OIDC Client Cred Auth"),
};

std::map<std::string, ApiAuthMethod> AUTH_NAME_TO_AUTH_METHOD = {
    std::make_pair("No Auth", AM_NO_AUTH),
    std::make_pair("External Auth", AM_EXTERNAL_AUTH),
    std::make_pair("Oidc Client Cred Auth", AM_CLIENT_CRED_AUTH),
};


// All default values - do not miss any!
std::map<std::string, std::string> DRIVER_CONFIG_DEFAULT_VALUES = {
    std::make_pair("hostname", "localhost"),
    std::make_pair("port", "8080"),
    std::make_pair("loglevel", "None"),
    std::make_pair("authmethod", "No Auth"),
    std::make_pair("oidcDiscoveryUrl", ""),
    std::make_pair("clientId", ""),
    std::make_pair("clientSecret", ""),
    std::make_pair("oidcScope", ""),
    std::make_pair("secretEncryptionLevel", "user"),
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

// IsSaved
bool DriverConfig::getIsSaved() {
  return this->isSaved;
}
void DriverConfig::setIsSaved(bool isSaved) {
  this->isSaved = isSaved;
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

  if (kvps.count("dsn")) {
    config.setDSN(kvps.at("dsn"));
  }
  if (kvps.count("driver")) {
    config.setDriver(kvps.at("driver"));
  }
  if (kvps.count("hostname")) {
    config.setHostname(kvps.at("hostname"));
  }
  if (kvps.count("port")) {
    config.setPort(kvps.at("port"));
  }
  if (kvps.count("loglevel")) {
    config.setLogLevel(kvps.at("loglevel"));
  }
  if (kvps.count("authmethod")) {
    config.setAuthMethod(kvps.at("authmethod"));
  }
  if (kvps.count("oidcDiscoveryUrl")) {
    config.setOidcDiscoveryUrl(kvps.at("oidcDiscoveryUrl"));
  }
  if (kvps.count("oidcdiscoveryurl")) {
    config.setOidcDiscoveryUrl(kvps.at("oidcdiscoveryurl"));
  }
  if (kvps.count("clientId")) {
    config.setClientId(kvps.at("clientId"));
  }
  if (kvps.count("clientid")) {
    config.setClientId(kvps.at("clientid"));
  }
  if (kvps.count("clientSecret")) {
    config.setClientSecret(kvps.at("clientSecret"));
  }
  if (kvps.count("clientsecret")) {
    config.setClientSecret(kvps.at("clientsecret"));
  }
  if (kvps.count("oidcScope")) {
    config.setOidcScope(kvps.at("oidcScope"));
  }
  if (kvps.count("oidcscope")) {
    config.setOidcScope(kvps.at("oidcscope"));
  }

  return config;
}

// Create key value pairs from a config.
std::map<std::string, std::string> driverConfigToKVPs(DriverConfig config) {
  std::map<std::string, std::string> kvps;

  if (!config.getDSN().empty()) {
    kvps["dsn"] = config.getDSN();
  }
  if (!config.getDriver().empty()) {
    kvps["driver"] = config.getDriver();
  }
  if (!config.getHostname().empty()) {
    kvps["hostname"] = config.getHostname();
  }
  if (!config.getPortStr().empty()) {
    kvps["port"] = config.getPortStr();
  }
  if (!config.getLogLevelStr().empty()) {
    kvps["loglevel"] = config.getLogLevelStr();
  }
  if (!config.getAuthMethodStr().empty()) {
    kvps["authmethod"] = config.getAuthMethodStr();
  }
  if (!config.getOidcDiscoveryUrl().empty()) {
    kvps["oidcDiscoveryUrl"] = config.getOidcDiscoveryUrl();
  }
  if (!config.getClientId().empty()) {
    kvps["clientId"] = config.getClientId();
  }
  if (!config.getClientSecret().empty()) {
    kvps["clientSecret"] = config.getClientSecret();
  }
  if (!config.getOidcScope().empty()) {
    kvps["oidcScope"] = config.getOidcScope();
  }

  return kvps;
}
