#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <iostream>

#include "../trinoAPIWrapper/environmentConfig.hpp"
#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"
#include "handles/descriptorHandle.hpp"
#include "handles/envHandle.hpp"
#include "handles/statementHandle.hpp"

using namespace std;

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
                                 SQLHANDLE InputHandle,
                                 _Out_ SQLHANDLE* OutputHandle) {
  /*
   * There are multiple types of handles in ODBC.
     - SQLHENV  = Environment Handle
     - SQLHDBC  = Connection Handle
     - SQLHSTMT = Statement Handle
     - SQLHDESC = DescriptorHandle

     This function needs to be able to allocate any of them.

     The InputHandle for Environment handles is ignored.
     The InputHandle for Connection handles is an environment handle.
     The InputHandle for Statement handles is a connection handle.
     The InputHandle for Descriptor handles could be either a connection or
   statement.
   */
  WriteLog(LL_TRACE, "Entering SQLAllocHandle");
  if (OutputHandle == NULL) {
    WriteLog(LL_ERROR, "  ERROR: allocHandle Output handle was null.");
    return SQL_INVALID_HANDLE;
  }

  switch (HandleType) {
    case SQL_HANDLE_ENV: {
      Environment* environment = new Environment();
      // Initialize the environment handle
      WriteLog(LL_TRACE, "  Initializing environment handle");

      *OutputHandle = reinterpret_cast<SQLHANDLE>(environment);
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_DBC: {
      if (InputHandle == SQL_NULL_HANDLE) {
        WriteLog(LL_ERROR, "  ERROR: Invalid environment handle.");
        return SQL_INVALID_HANDLE;
      }
      WriteLog(LL_TRACE, "  Constructing connection handle");
      Environment* environment = reinterpret_cast<Environment*>(InputHandle);
      Connection* connection   = new Connection(environment->environmentConfig);
      *OutputHandle            = reinterpret_cast<SQLHANDLE>(connection);
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_STMT: {
      if (InputHandle == SQL_NULL_HANDLE) {
        WriteLog(LL_ERROR, "  ERROR: Invalid connection handle.");
        return SQL_INVALID_HANDLE;
      }
      WriteLog(LL_TRACE, "  Constructing statement handle");
      Connection* connection = reinterpret_cast<Connection*>(InputHandle);
      Statement* statement   = new Statement(connection->connectionConfig);
      *OutputHandle          = reinterpret_cast<SQLHANDLE>(statement);
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_DESC: {
      if (InputHandle == SQL_NULL_HANDLE) {
        WriteLog(LL_ERROR, "  ERROR: Invalid input handle.");
        return SQL_INVALID_HANDLE;
      }

      Descriptor* descriptor = new Descriptor();
      // This driver doesn't call allocHandle, so if it gets called,
      // the application (user) must be allocating this descriptor.
      descriptor->Field_AllocType = SQL_DESC_ALLOC_USER;
      WriteLog(LL_TRACE, "  Initializing descriptor handle");
      *OutputHandle = reinterpret_cast<SQLHANDLE>(descriptor);
      return SQL_SUCCESS;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unknown handle type");
      return SQL_ERROR;
    }
  }
}


SQLRETURN SQL_API SQLAllocEnv(_Out_ SQLHENV* EnvironmentHandle) {
  // A deprecated ODBC2.0 version
  WriteLog(LL_WARN, "Entering SQLAllocEnv (deprecated)");
  return SQLAllocHandle(SQL_HANDLE_ENV, nullptr, EnvironmentHandle);
}

SQLRETURN SQL_API SQLAllocConnect(SQLHENV EnvironmentHandle,
                                  _Out_ SQLHDBC* ConnectionHandle) {
  // A deprecated ODBC2.0 version
  WriteLog(LL_WARN, "Entering SQLAllocConnect (deprecated)");
  return SQLAllocHandle(SQL_HANDLE_DBC, EnvironmentHandle, ConnectionHandle);
}

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC ConnectionHandle,
                               _Out_ SQLHSTMT* StatementHandle) {
  // A deprecated ODBC2.0 version
  WriteLog(LL_WARN, "Entering SQLAllocStmt (deprecated)");
  return SQLAllocHandle(SQL_HANDLE_STMT, ConnectionHandle, StatementHandle);
}
