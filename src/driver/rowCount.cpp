#include "../util/windowsLean.hpp"
#include <sql.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLRowCount(_In_ SQLHSTMT StatementHandle,
                              _Out_ SQLLEN* RowCount) {
  WriteLog(LL_TRACE, "Entering SQLRowCount");

  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  // We can return -1 to indicate that we do not
  // yet know the number of rows in the query. This is
  // useful because this will often be called even before
  // the first execution of SQLFetch.
  // getAbsoluteRowCount will return -1 if the query is not yet complete
  SQLLEN queryRowCount =
      static_cast<SQLLEN>(statement->trinoQuery->getAbsoluteRowCount());
  *RowCount = queryRowCount;

  WriteLog(LL_TRACE, "  Row count is set to: " + std::to_string(*RowCount));
  return SQL_SUCCESS;
}
