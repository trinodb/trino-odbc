#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ipar,
                                   SQLSMALLINT fParamType,
                                   SQLSMALLINT fCType,
                                   SQLSMALLINT fSqlType,
                                   SQLULEN cbColDef,
                                   SQLSMALLINT ibScale,
                                   SQLPOINTER rgbValue,
                                   SQLLEN cbValueMax,
                                   SQLLEN* pcbValue) {
  WriteLog(LL_TRACE, "Entering SQLBindParameter");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  // This could probably look a lot like the SQLBindCol function.
  WriteLog(LL_ERROR, "  ERROR: SQLBindParameter is unimplemented");
  return SQL_ERROR;
}
