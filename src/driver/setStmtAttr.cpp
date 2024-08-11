#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../trinoAPIWrapper/trinoQuery.hpp"
#include "../util/writeLog.hpp"
#include "constants/statementAttrs.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT StatementHandle,
                                 SQLINTEGER Attribute,
                                 _In_reads_(_Inexpressible_(StringLength))
                                     SQLPOINTER Value,
                                 SQLINTEGER StringLength) {
  WriteLog(LL_TRACE, "Entering SQLSetStmtAttr");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  WriteLog(LL_TRACE, "  Setting attribute: " + std::to_string(Attribute));
  switch (Attribute) {
    case SQL_ATTR_ROWS_FETCHED_PTR: { // 26
      SQLULEN* rowsProcessedPtr = static_cast<SQLULEN*>(Value);
      WriteLog(LL_TRACE, std::format("  Attribute value is set to {}", Value));
      Descriptor* impRowDesc             = statement->impRowDesc;
      impRowDesc->Field_RowsProcessedPtr = rowsProcessedPtr;
      break;
    }
    case SQL_ATTR_DEFAULT_FETCH_POLL_MODE: { // 1002
      SQLINTEGER pollModeInt = *reinterpret_cast<SQLINTEGER*>(Value);
      WriteLog(LL_TRACE,
               "  Attribute value is set to " + std::to_string(pollModeInt));
      statement->fetchPollMode = static_cast<TrinoQueryPollMode>(pollModeInt);
      break;
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: Attribute " + std::to_string(Attribute) +
                   " is not implemented");
      return SQL_ERROR;
    }
  }

  return SQL_SUCCESS;
}
