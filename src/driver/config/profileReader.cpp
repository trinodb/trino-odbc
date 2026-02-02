#include "profileReader.hpp"

#include "../../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <cstdint>
#include <odbcinst.h>

#include "../../util/cryptUtils.hpp"
#include "../../util/writeLog.hpp"
#include "driverConfig.hpp"


std::string readFromPrivateProfile(std::string dsn, std::string key) {
  // Big buffer to hold values pulled out of the profile.
  char value[2048];

  std::string defaultVal = "";
  if (DRIVER_CONFIG_DEFAULT_VALUES.count(key)) {
    defaultVal = DRIVER_CONFIG_DEFAULT_VALUES.at(key);
  }

  SQLGetPrivateProfileString(dsn.c_str(),
                             key.c_str(),
                             defaultVal.c_str(),
                             value,
                             sizeof(value),
                             "ODBC.INI");

  return std::string(value);
}

DriverConfig readDriverConfigFromProfile(std::string dsn) {
  // The driver config reads values assuming mostly camelCase names.
  DriverConfig config;
  config.setDSN(dsn);
  config.setHostname(readFromPrivateProfile(dsn, CONFIG_FIELD_HOSTNAME));
  config.setPort(readFromPrivateProfile(dsn, CONFIG_FIELD_PORT));
  config.setLogLevel(readFromPrivateProfile(dsn, CONFIG_FIELD_LOGLEVEL));
  config.setAuthMethod(readFromPrivateProfile(dsn, CONFIG_FIELD_AUTHMETHOD));
  config.setOidcDiscoveryUrl(readFromPrivateProfile(dsn, CONFIG_FIELD_OIDCDISCOVERYURL));
  config.setClientId(readFromPrivateProfile(dsn, CONFIG_FIELD_CLIENTID));
  config.setOidcScope(readFromPrivateProfile(dsn, CONFIG_FIELD_OIDCSCOPE));
  config.setAuthEndpoint(
      readFromPrivateProfile(dsn, CONFIG_FIELD_AUTHENDPOINT));
  config.setPassword(readFromPrivateProfile(dsn, CONFIG_FIELD_PASSWORD));
  config.setUsername(readFromPrivateProfile(dsn, CONFIG_FIELD_USERNAME));
  config.setTokenEndpoint(
      readFromPrivateProfile(dsn, CONFIG_FIELD_TOKENENDPOINT));
  config.setOidcEndpointMethod(readFromPrivateProfile(dsn, CONFIG_FIELD_OIDCDISCOVERYMETHOD));

  std::string secretEncryptionLevel =
      readFromPrivateProfile(dsn, CONFIG_FIELD_SECRETENCRYPTIONLEVEL);
  std::string encryptedClientSecret =
      readFromPrivateProfile(dsn, CONFIG_FIELD_ENCRYPTEDCLIENTSECRET);

  // Handle the encrypted data
  if (secretEncryptionLevel == "user") {
    config.setClientSecret(userDecryptString(encryptedClientSecret));
  } else if (secretEncryptionLevel == "system") {
    config.setClientSecret(systemDecryptString(encryptedClientSecret));
  } else {
    throw std::runtime_error("Unknown secret encryption level: " +
                             secretEncryptionLevel);
  }

  return config;
}
