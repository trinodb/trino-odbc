#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLCancel(SQLHSTMT StatementHandle) {
  WriteLog(LL_TRACE, "Entering SQLCancel");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  WriteLog(LL_INFO, "  Canceling current trino query");
  // It seems like statement->trinoQuery->cancel() would make more sense
  // here, but in my testing, terminating the query seemed
  // to behave better. It stopped it instantly, and showed
  // USER_CANCELED as the status in the Trino UI.
  statement->terminate();
  WriteLog(LL_INFO, "  Query cancellation completed");
  return SQL_SUCCESS;
}
