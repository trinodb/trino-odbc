#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT StatementHandle) {
  WriteLog(LL_TRACE, "Entering SQLCloseCursor");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  WriteLog(LL_ERROR, "  ERROR: SQLCloseCursor is unimplemented");
  return SQL_ERROR;
}
