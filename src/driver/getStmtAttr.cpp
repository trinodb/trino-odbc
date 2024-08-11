#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "constants/statementAttrs.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLGetStmtAttr(SQLHSTMT StatementHandle,
                                 SQLINTEGER Attribute,
                                 _Out_writes_opt_(_Inexpressible_(BufferLength))
                                     SQLPOINTER Value,
                                 SQLINTEGER BufferLength,
                                 _Out_opt_ SQLINTEGER* StringLength) {
  WriteLog(LL_TRACE, "Entering SQLGetStmtAttr");
  if (!StatementHandle) {
    WriteLog(LL_ERROR, "  ERROR: Invalid statement handle");
    if (Value) {
      Value = nullptr;
    }
    if (StringLength) {
      StringLength = 0;
    }
    return SQL_INVALID_HANDLE;
  }

  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  switch (Attribute) {
    case SQL_ATTR_ROW_NUMBER: { // 14
      if (Value) {
        *reinterpret_cast<SQLULEN*>(Value) = statement->getFetchedPosition();
      }
      if (StringLength) {
        *StringLength = sizeof(SQLULEN*);
      }
      break;
    }
    case SQL_ATTR_APP_ROW_DESC: { // 10010
      if (Value) {
        *reinterpret_cast<SQLPOINTER*>(Value) = statement->appRowDesc;
      }
      if (StringLength) {
        *StringLength = sizeof(SQLPOINTER*);
      }
      break;
    }
    case SQL_ATTR_APP_PARAM_DESC: { // 10011
      if (Value) {
        *reinterpret_cast<SQLPOINTER*>(Value) = statement->appParamDesc;
      }
      if (StringLength) {
        *StringLength = sizeof(SQLPOINTER*);
      }
      break;
    }
    case SQL_ATTR_IMP_ROW_DESC: { // 10012
      if (Value) {
        *reinterpret_cast<SQLPOINTER*>(Value) = statement->impRowDesc;
      }
      if (StringLength) {
        *StringLength = sizeof(SQLPOINTER*);
      }
      break;
    }
    case SQL_ATTR_IMP_PARAM_DESC: { // 10013
      if (Value) {
        *reinterpret_cast<SQLPOINTER*>(Value) = statement->impParamDesc;
      }
      if (StringLength) {
        *StringLength = sizeof(SQLPOINTER*);
      }
      break;
    }
    case SQL_ATTR_RAW_STATEMENT_HANDLE: { // 1001
      // Driver defined - get raw statement handle, no ODBC proxy.
      if (Value) {
        *reinterpret_cast<SQLPOINTER*>(Value) = StatementHandle;
      }
      if (StringLength) {
        *StringLength = sizeof(SQLPOINTER*);
      }
      break;
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: Unsupported attribute: " + std::to_string(Attribute));
      if (Value) {
        memset(Value, 0, BufferLength);
      }
      if (StringLength) {
        *StringLength = 0;
      }
      ErrorInfo errorInfo("Unknown Statement Attribute", "HY092");
      statement->setError(errorInfo);
      return SQL_ERROR;
    }
  }
  WriteLog(LL_TRACE,
           "  Finished getting attribute: " + std::to_string(Attribute));
  return SQL_SUCCESS;
}
