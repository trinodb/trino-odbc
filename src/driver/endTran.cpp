#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT HandleType,
                             SQLHANDLE Handle,
                             SQLSMALLINT CompletionType) {
  WriteLog(LL_TRACE, "Entering SQLEndTran");
  /*
  Trino supplies HTTP headers to accomplish this.
  We need to parse/handle them to enable this functionality.
  * `X-Trino-Transaction-Id`
  * `X-Trino-Stared-Transaction-Id`
  * `X-Trino-Clear-Transaction-Id`
  */
  return SQL_SUCCESS;
}
