#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLCancelHandle(SQLSMALLINT HandleType,
                                  SQLHANDLE InputHandle) {
  WriteLog(LL_TRACE, "Entering SQLCancelHandle");
  switch (HandleType) {
    case SQL_HANDLE_STMT: {
      // According to the docs, the driver manager will automatically
      // translate this to a call to SQLCancel instead, but let's
      // write the code anyway.
      WriteLog(LL_WARN, "  Canceling statement handle");
      Statement* statement = reinterpret_cast<Statement*>(InputHandle);
      statement->terminate();
      break;
    }
    case SQL_HANDLE_DBC: {
      WriteLog(LL_TRACE, "  Canceling connection handle");
      Connection* conn = reinterpret_cast<Connection*>(InputHandle);
      conn->disconnect();
    }
  }
  return SQL_SUCCESS;
}
