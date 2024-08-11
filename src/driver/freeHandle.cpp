#include "../util/windowsLean.hpp"
#include <sql.h>

#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"
#include "handles/descriptorHandle.hpp"
#include "handles/envHandle.hpp"
#include "handles/statementHandle.hpp"

using namespace std;

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle) {
  WriteLog(LL_TRACE, "Entering SQLFreeHandle");
  if (Handle == SQL_NULL_HANDLE) {
    WriteLog(LL_ERROR, "  ERROR: Invalid handle in SQLFreeHandle");
    return SQL_INVALID_HANDLE;
  }

  switch (HandleType) {
    case SQL_HANDLE_ENV: {
      Environment* env = reinterpret_cast<Environment*>(Handle);
      // Perform any environment-specific cleanup here
      WriteLog(LL_TRACE, "  Freeing environment handle");
      delete env;
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_DBC: {
      Connection* dbc = reinterpret_cast<Connection*>(Handle);
      // It's nice to run a disconnect before cleaning up the
      // connection handle. Usually this will do nothing, but
      // it can't hurt since it does nothing to a properly
      // closed connection that no longer has any live statements.
      dbc->disconnect();
      WriteLog(LL_TRACE, "  Freeing connection handle");
      delete dbc;
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_STMT: {
      Statement* stmt = reinterpret_cast<Statement*>(Handle);
      WriteLog(LL_TRACE, "  Freeing statement handle");
      // Some clients free the statement handle without terminating
      // a running query first. That means we need to be smart here
      // and assume the statement handle is not yet in a state that
      // is ready to be freed. We can prevent it from leaving Trino
      // queries hanging in the `FINISHING` state by doing a terminate()
      // before we free the statement. Conveniently, that does nothing
      // if a query is not currently running.
      stmt->terminate();
      delete stmt;
      return SQL_SUCCESS;
    }

    case SQL_HANDLE_DESC: {
      Descriptor* desc = reinterpret_cast<Descriptor*>(Handle);
      // Perform any descriptor-specific cleanup here
      WriteLog(LL_TRACE, "  Freeing descriptor handle");
      delete desc;
      return SQL_SUCCESS;
    }

    default: {
      WriteLog(LL_TRACE, "  ERROR: Unknown handle type in SQLFreeHandle");
      return SQL_ERROR;
    }
  }
}

SQLRETURN SQL_API SQLFreeEnv(SQLHENV EnvironmentHandle) {
  WriteLog(LL_WARN, "Entering SQLFreeEnv (deprecated)");
  return SQLFreeHandle(SQL_HANDLE_ENV, EnvironmentHandle);
}

SQLRETURN SQL_API SQLFreeConnect(SQLHDBC ConnectionHandle) {
  WriteLog(LL_WARN, "Entering SQLFreeConnect (deprecated)");
  return SQLFreeHandle(SQL_HANDLE_DBC, ConnectionHandle);
}
