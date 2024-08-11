#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/envHandle.hpp"

SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV EnvironmentHandle,
                                SQLINTEGER Attribute,
                                _Out_writes_(_Inexpressible_(BufferLength))
                                    SQLPOINTER Value,
                                SQLINTEGER BufferLength,
                                _Out_opt_ SQLINTEGER* StringLength) {
  WriteLog(LL_TRACE, "Entering SQLGetEnvAttr");

  if (!EnvironmentHandle) {
    WriteLog(LL_ERROR, "  ERROR: EnvironmentHandle is invalid.");
    if (Value) {
      Value = nullptr;
    }
    if (StringLength) {
      *StringLength = 0;
    }
    return SQL_ERROR;
  }

  Environment* environment = reinterpret_cast<Environment*>(EnvironmentHandle);

  // It looks like several of these are reinterpreting a non-pointer as a
  // void pointer. They are! This is how SQLGetEnvAttr returns numeric
  // values. For string values, the pointer is treated as an actual pointer.
  switch (Attribute) {
    case SQL_ATTR_CONNECTION_POOLING: {
      SQLINTEGER pooling = environment->AttrConnectionPooling;
      if (Value) {
        Value = reinterpret_cast<void*>(static_cast<uintptr_t>(pooling));
      }
      if (StringLength) {
        *StringLength = sizeof(void*);
      }
      WriteLog(LL_TRACE,
               "  Connection pooling read as: " + std::to_string(pooling));
      break;
    }
    case SQL_ATTR_ODBC_VERSION: {
      SQLINTEGER version = environment->AttrODBCVersion;
      if (Value) {
        Value = reinterpret_cast<void*>(static_cast<uintptr_t>(version));
      }
      if (StringLength) {
        *StringLength = sizeof(void*);
      }
      WriteLog(LL_TRACE, "  ODBC Version read as: " + std::to_string(version));
      break;
    }
    default: {
      if (Value) {
        memset(Value, 0, BufferLength);
      }
      if (StringLength) {
        *StringLength = 0;
      }
      WriteLog(LL_ERROR, "  ERROR: Unsupported attribute in SQLGetEnvAttr.");
      WriteLog(LL_ERROR, "  Attribute is " + std::to_string(Attribute));
      return SQL_ERROR;
    }
  }
  WriteLog(LL_TRACE, "Successful environment attribute getting.");
  return SQL_SUCCESS;
}
