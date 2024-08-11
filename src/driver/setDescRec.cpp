#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLSetDescRec(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber,
                                SQLSMALLINT Type,
                                SQLSMALLINT SubType,
                                SQLLEN Length,
                                SQLSMALLINT Precision,
                                SQLSMALLINT Scale,
                                _Inout_updates_bytes_opt_(Length)
                                    SQLPOINTER Data,
                                _Inout_opt_ SQLLEN* StringLength,
                                _Inout_opt_ SQLLEN* Indicator) {
  WriteLog(LL_TRACE, "Entering SQLSetDescRec");
  WriteLog(LL_ERROR, "  ERROR: SQLSetDescRec is unimplemented");
  return SQL_ERROR;
}
