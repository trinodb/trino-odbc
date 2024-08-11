#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT hstmt,
                                   SQLUSMALLINT fFetchType,
                                   SQLLEN irow,
                                   _Out_opt_ SQLULEN* pcrow,
                                   _Out_opt_ SQLUSMALLINT* rgfRowStatus) {
  WriteLog(LL_TRACE, "Entering SQLExtendedFetch");
  WriteLog(LL_ERROR, "  ERROR: SQLExtendedFetch unimplemented");
  return SQL_ERROR;
}
