#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "../../trinoAPIWrapper/apiAuthMethod.hpp"
#include "../../util/writeLog.hpp"


extern std::map<std::string, std::string> DRIVER_CONFIG_DEFAULT_VALUES;
extern std::vector<std::string> LOG_LEVEL_NAMES;
extern std::vector<std::string> AUTH_METHOD_NAMES;

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
};

DriverConfig driverConfigFromKVPs(std::map<std::string, std::string> kvps);
std::map<std::string, std::string> driverConfigToKVPs(DriverConfig config);
