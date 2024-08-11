#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"

SQLRETURN SQL_API SQLDisconnect(SQLHDBC ConnectionHandle) {
  WriteLog(LL_TRACE, "Entering SQLDisconnect");
  Connection* connection = reinterpret_cast<Connection*>(ConnectionHandle);
  connection->disconnect();
  return SQL_SUCCESS;
}
