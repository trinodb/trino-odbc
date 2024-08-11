#include "../util/windowsLean.hpp"
#include <sql.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT StatementHandle,
                                   _Out_ SQLSMALLINT* ColumnCount) {
  WriteLog(LL_TRACE, "Entering SQLNumResultCols");

  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  WriteLog(LL_TRACE, "  Getting Column Count");
  SQLSMALLINT queryColumnCount = statement->trinoQuery->getColumnCount();
  WriteLog(LL_TRACE, "  Got Column Count");
  *ColumnCount = queryColumnCount;
  WriteLog(LL_TRACE,
           "  Result col count is set to: " + std::to_string(*ColumnCount));

  return SQL_SUCCESS;
}
