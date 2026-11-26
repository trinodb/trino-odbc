#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>

#include "config/profileReader.hpp"
#include "handles/connHandle.hpp"

#include "../util/stringFromChar.hpp"
#include "../util/writeLog.hpp"


SQLRETURN SQL_API SQLConnect(SQLHDBC ConnectionHandle,
                             _In_reads_(NameLength1) SQLCHAR* DSNChars,
                             SQLSMALLINT NameLength1,
                             _In_reads_(NameLength2) SQLCHAR* UserNameChars,
                             SQLSMALLINT NameLength2,
                             _In_reads_(NameLength3)
                                 SQLCHAR* AuthenticationChars,
                             SQLSMALLINT NameLength3) {
  WriteLog(LL_TRACE, "Entering SQLConnect");
  std::string dsn        = stringFromChar(DSNChars, NameLength1);
  Connection* connection = reinterpret_cast<Connection*>(ConnectionHandle);
  DriverConfig config    = readDriverConfigFromProfile(dsn);

  // SQLConnect passes the user name and password as separate arguments.
  if (UserNameChars != nullptr) {
    std::string user = stringFromChar(UserNameChars, NameLength2);
    if (!user.empty()) {
      config.setUser(user);
    }
  }
  if (AuthenticationChars != nullptr) {
    std::string password = stringFromChar(AuthenticationChars, NameLength3);
    if (!password.empty()) {
      config.setPassword(password);
    }
  }

  // It's kind of unfortunate that we can't set the log level of the driver
  // until we've read the DSN in some way.
  WriteLog(LL_TRACE, "  Setting Log Level");
  setLogLevel(config.getLogLevelEnum());

  WriteLog(LL_TRACE, "  Configuring connection");
  try {
    connection->configure(config);
  } catch (std::exception& e) {
    WriteLog(LL_ERROR, "  Error from SQLConnect: " + std::string(e.what()));
    connection->setError(ErrorInfo(e.what(), "HY000"));
    return SQL_ERROR;
  }
  connection->connected = true;

  return SQL_SUCCESS;
}
