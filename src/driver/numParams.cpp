#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLNumParams(SQLHSTMT hstmt, _Out_opt_ SQLSMALLINT* pcpar) {
  WriteLog(LL_TRACE, "Entering SQLNumParams");
  WriteLog(LL_ERROR, " ERROR: SQLNumParams is unimplemented");
  return SQL_ERROR;
}
