#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLPrepare(SQLHSTMT StatementHandle,
                             _In_reads_(TextLength) SQLCHAR* StatementText,
                             SQLINTEGER TextLength) {
  WriteLog(LL_TRACE, "Entering SQLPrepare");
  WriteLog(LL_ERROR, "  ERROR: SQLPrepare is unimplemented");
  return SQL_ERROR;
}
