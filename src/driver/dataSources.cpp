#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLDataSources(SQLHENV EnvironmentHandle,
                                 SQLUSMALLINT Direction,
                                 _Out_writes_opt_(BufferLength1)
                                     SQLCHAR* ServerName,
                                 SQLSMALLINT BufferLength1,
                                 _Out_opt_ SQLSMALLINT* NameLength1Ptr,
                                 _Out_writes_opt_(BufferLength2)
                                     SQLCHAR* Description,
                                 SQLSMALLINT BufferLength2,
                                 _Out_opt_ SQLSMALLINT* NameLength2Ptr) {
  WriteLog(LL_TRACE, "Entering SQLDataSources");
  WriteLog(LL_ERROR, "  ERROR: SQLDataSources is unimplemented");
  return SQL_ERROR;
}
