#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLExecute(SQLHSTMT StatementHandle) {
  WriteLog(LL_TRACE, "Entering SQLExecute");
  WriteLog(LL_ERROR, "  ERROR: SQLExecute unimplemented");
  return SQL_ERROR;
}
