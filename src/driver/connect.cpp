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

  // It's kind of unfortunate that we can't set the log level of the driver
  // until we've read the DSN in some way.
  WriteLog(LL_TRACE, "  Setting Log Level");
  setLogLevel(config.getLogLevelEnum());

  WriteLog(LL_TRACE, "  Configuring connection");
  connection->configure(config);

  return SQL_SUCCESS;
}
