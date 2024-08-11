#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLStatistics(SQLHSTMT StatementHandle,
                                _In_reads_opt_(NameLength1)
                                    SQLCHAR* CatalogName,
                                SQLSMALLINT NameLength1,
                                _In_reads_opt_(NameLength2) SQLCHAR* SchemaName,
                                SQLSMALLINT NameLength2,
                                _In_reads_opt_(NameLength3) SQLCHAR* TableName,
                                SQLSMALLINT NameLength3,
                                SQLUSMALLINT Unique,
                                SQLUSMALLINT Reserved) {
  WriteLog(LL_TRACE, "Entering SQLStatistics");
  WriteLog(LL_ERROR, "  ERROR: SQLStatistics is unimplemented");
  return SQL_ERROR;
}
