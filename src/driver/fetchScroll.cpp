#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLFetchScroll(SQLHSTMT StatementHandle,
                                 SQLSMALLINT FetchOrientation,
                                 SQLLEN FetchOffset) {
  WriteLog(LL_TRACE, "Entering SQLFetchScroll");
  WriteLog(LL_ERROR, "  ERROR: SQLFetchScroll is unimplemented");
  return SQL_ERROR;
}
