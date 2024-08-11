#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLMoreResults(SQLHSTMT StatementHandle) {
  WriteLog(LL_TRACE, "Entering SQLMoreResults");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  /*
  I don't think Trino supports returning multiple result sets
  at a time via the REST API. We'll just implement the behavior
  that there is never another result set.

  One interesting behavior of SQLMoreResults is that if there is
  a current result set running with unfetched rows, SQLMoreResults
  will discard that result set, so this works as a kind of
  "terminate" function for existing queries. We do want to copy
  that behavior.
  */
  statement->terminate();
  return SQL_NO_DATA;
}
