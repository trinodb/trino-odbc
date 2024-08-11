#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT StatementHandle, SQLUSMALLINT Option) {
  WriteLog(LL_TRACE, "Entering SQLFreeStmt");
  Statement* stmt = reinterpret_cast<Statement*>(StatementHandle);
  switch (Option) {
    case (SQL_CLOSE): {
      WriteLog(
          LL_TRACE,
          "  Closing statement with SQL_CLOSE. The statement may be reused.");
      stmt->reset();
      return SQL_SUCCESS;
    }
    case (SQL_DROP): {
      WriteLog(LL_WARN, "  Closing statement with SQL_DROP (deprecated)");
      return SQLFreeHandle(SQL_HANDLE_STMT, StatementHandle);
      return SQL_SUCCESS;
    }
    case (SQL_UNBIND): {
      WriteLog(LL_ERROR,
               "  ERROR: Unimplemented: Closing statement with SQL_UNBIND");
      return SQL_ERROR;
    }
    case (SQL_RESET_PARAMS): {
      WriteLog(
          LL_ERROR,
          "  ERROR: Unimplemented: Closing statement with SQL_RESET_PARAMS");
      return SQL_ERROR;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unknown option in SQLFreeStmt");
      return SQL_ERROR;
    }
  }
}
