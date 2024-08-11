#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"

SQLRETURN SQL_API SQLSetConnectOption(SQLHDBC ConnectionHandle,
                                      SQLUSMALLINT Option,
                                      SQLULEN Value) {
  // A deprecated ODBC2.0 version
  WriteLog(LL_WARN, "Entering SQLSetConnectOption (deprecated)");
  WriteLog(LL_WARN, "  Setting option: " + std::to_string(Option));
  WriteLog(LL_WARN, "  To value: " + std::to_string(Value));

  WriteLog(LL_ERROR, "  ERROR - Returning error from SQLSetConnectOption");
  return SQL_ERROR;
}

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC ConnectionHandle,
                                    SQLINTEGER Attribute,
                                    _In_reads_bytes_opt_(StringLength)
                                        SQLPOINTER Value,
                                    SQLINTEGER StringLength) {
  /*
  The Value parameter claims to be a SQLPOINTER, but it is only a pointer
  in the event that the attribute that is being set is a string type attribute.
  When setting integer type attributes such as SQL_ATTR_AUTOCOMMIT, SQLPOINTER
  is not a pointer, it's a 32 bit integer that is passed by value.
  */

  WriteLog(LL_TRACE, "Entering SQLSetConnectAttr");
  Connection* connection = reinterpret_cast<Connection*>(ConnectionHandle);
  WriteLog(LL_TRACE,
           "  Request to set attribute: " + std::to_string(Attribute));

  if (connection == nullptr) {
    WriteLog(LL_ERROR, "  ERROR: ConnectionHandle is invalid.");
    return SQL_ERROR;
  }

  switch (Attribute) {
    case SQL_ATTR_AUTOCOMMIT: { // 102
      SQLINTEGER autocommitMode =
          static_cast<SQLINTEGER>(reinterpret_cast<std::uintptr_t>(Value));
      connection->ATTR_AutoCommitMode = autocommitMode;
      WriteLog(LL_TRACE,
               "  Autocommit mode set to: " + std::to_string(autocommitMode));
      break;
    }
    case SQL_ATTR_LOGIN_TIMEOUT: { // 103
      SQLUINTEGER loginTimeout =
          static_cast<SQLUINTEGER>(reinterpret_cast<std::uintptr_t>(Value));
      connection->ATTR_LoginTimeout = loginTimeout;
      WriteLog(LL_TRACE,
               "  Login timeout set to: " + std::to_string(loginTimeout));
      break;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unsupported attribute in SetConnectAttr.");
      WriteLog(LL_ERROR, "  Attribute is " + std::to_string(Attribute));
      return SQL_ERROR;
    }
  }
  WriteLog(LL_TRACE, "  Successful connect attribute setting.");
  return SQL_SUCCESS;
}
