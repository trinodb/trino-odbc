#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT StatementHandle,
                                   _Out_writes_opt_(BufferLength)
                                       SQLCHAR* CursorName,
                                   SQLSMALLINT BufferLength,
                                   _Out_opt_ SQLSMALLINT* NameLengthPtr) {
  WriteLog(LL_TRACE, "Entering SQLGetCursorName");
  WriteLog(LL_ERROR, "  ERROR: SQLGetCursorName unimplemented");
  return SQL_ERROR;
}
