#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLDrivers(SQLHENV henv,
                             SQLUSMALLINT fDirection,
                             _Out_writes_opt_(cchDriverDescMax)
                                 SQLCHAR* szDriverDesc,
                             SQLSMALLINT cchDriverDescMax,
                             _Out_opt_ SQLSMALLINT* pcchDriverDesc,
                             _Out_writes_opt_(cchDrvrAttrMax)
                                 SQLCHAR* szDriverAttributes,
                             SQLSMALLINT cchDrvrAttrMax,
                             _Out_opt_ SQLSMALLINT* pcchDrvrAttr) {
  WriteLog(LL_TRACE, "Entering SQLDrivers");
  WriteLog(LL_ERROR, "  ERROR: This is not implemented");
  return SQL_ERROR;
}
