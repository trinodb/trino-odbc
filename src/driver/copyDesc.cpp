#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLCopyDesc(SQLHDESC SourceDescHandle,
                              SQLHDESC TargetDescHandle) {
  WriteLog(LL_TRACE, "Entering SQLCopyDesc");

  WriteLog(LL_ERROR, "  ERROR: SQLCopyDesc is unimplemented");
  return SQL_ERROR;
}
