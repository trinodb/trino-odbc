#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLNativeSql(SQLHDBC hdbc,
                               _In_reads_(cchSqlStrIn) SQLCHAR* szSqlStrIn,
                               SQLINTEGER cchSqlStrIn,
                               _Out_writes_opt_(cchSqlStrMax) SQLCHAR* szSqlStr,
                               SQLINTEGER cchSqlStrMax,
                               SQLINTEGER* pcbSqlStr) {
  WriteLog(LL_TRACE, "Entering SQLNativeSQL");
  WriteLog(LL_ERROR, "  ERROR: SQLNativeSQL is unimplemented");
  return SQL_ERROR;
}
