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
  config.setHostname(readFromPrivateProfile(dsn, "hostname"));
  config.setPort(readFromPrivateProfile(dsn, "port"));
  config.setLogLevel(readFromPrivateProfile(dsn, "loglevel"));
  config.setAuthMethod(readFromPrivateProfile(dsn, "authmethod"));
  config.setOidcDiscoveryUrl(readFromPrivateProfile(dsn, "oidcDiscoveryUrl"));
  config.setClientId(readFromPrivateProfile(dsn, "clientId"));
  config.setOidcScope(readFromPrivateProfile(dsn, "oidcScope"));

  std::string secretEncryptionLevel =
      readFromPrivateProfile(dsn, "secretEncryptionLevel");
  std::string encryptedClientSecret =
      readFromPrivateProfile(dsn, "encryptedClientSecret");

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
