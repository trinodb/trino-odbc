#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/envHandle.hpp"

SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV EnvironmentHandle,
                                SQLINTEGER Attribute,
                                _In_reads_bytes_opt_(StringLength)
                                    SQLPOINTER Value,
                                SQLINTEGER StringLength) {
  /*
  The Value parameter claims to be a SQLPOINTER, but it is only a pointer
  in the event that the attribute that is being set is a string type attribute.
  When setting integer type attributes such as SQL_ATTR_ODBC_VERSION, SQLPOINTER
  is not a pointer, it's an integer that is passed by value.
  */

  WriteLog(LL_TRACE, "Entering SQLSetEnvAttr");
  Environment* environment = reinterpret_cast<Environment*>(EnvironmentHandle);

  if (environment == nullptr) {
    WriteLog(LL_ERROR, "  ERROR: EnvironmentHandle is invalid.");
    return SQL_ERROR;
  }

  // It looks like several of these are reinterpreting a void pointer as a
  // non-pointer, and losing precision on 64 bit systems in the process.
  // It is! See the comment about the value parameter above.
  switch (Attribute) {
    case SQL_ATTR_CONNECTION_POOLING: {
      SQLINTEGER pooling =
          static_cast<SQLINTEGER>(reinterpret_cast<std::intptr_t>(Value));
      environment->AttrConnectionPooling = pooling;
      WriteLog(LL_TRACE,
               "  Connection pooling set to: " + std::to_string(pooling));
      break;
    }
    case SQL_ATTR_ODBC_VERSION: {
      SQLINTEGER version =
          static_cast<SQLINTEGER>(reinterpret_cast<std::intptr_t>(Value));
      environment->AttrODBCVersion = version;
      WriteLog(LL_TRACE, "  ODBC Version set to: " + std::to_string(version));
      break;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unsupported attribute in SetEnvAttr.");
      WriteLog(LL_ERROR, "  Attribute is " + std::to_string(Attribute));
      return SQL_ERROR;
    }
  }
  WriteLog(LL_TRACE, "  Successful environment attribute setting.");
  return SQL_SUCCESS;
}
