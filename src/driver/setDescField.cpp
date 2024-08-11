#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLSetDescField(SQLHDESC DescriptorHandle,
                                  SQLSMALLINT RecNumber,
                                  SQLSMALLINT FieldIdentifier,
                                  _In_reads_(_Inexpressible_(BufferLength))
                                      SQLPOINTER Value,
                                  SQLINTEGER BufferLength) {
  WriteLog(LL_TRACE, "Entering SQLSetDescField");
  WriteLog(LL_ERROR, " ERROR: SQLSetDescField is unimplemented");
  return SQL_ERROR;
}
