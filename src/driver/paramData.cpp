#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLParamData(SQLHSTMT StatementHandle,
                               _Out_opt_ SQLPOINTER* Value) {
  WriteLog(LL_TRACE, "Entering SQLParamData");
  WriteLog(LL_ERROR, "  ERROR: SQLParamData is unimplemented");
  return SQL_ERROR;
}
