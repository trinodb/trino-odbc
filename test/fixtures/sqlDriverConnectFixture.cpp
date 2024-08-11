#include "sqlDriverConnectFixture.hpp"

#include "../constants.hpp"

void SQLDriverConnectFixture::SetUp() {
  return this->SetUp("");
}

void SQLDriverConnectFixture::SetUp(std::string extraConnStr) {
  // PyODBC does this before allocating an environment handle.
  // We will do it as well for this test.
  SQLRETURN ret = SQLSetEnvAttr(
      nullptr, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_HENV, 4);

  // Allocate environment handle
  ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLAllocHandle an env";

  // Set the ODBC version environment attribute
  ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to set ODBCVer env attr";

  // Allocate connection handle
  ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLAllocHandle a dbc";

  // Connect to the data source
  std::string inConnectionStr = "DSN=" + TEST_DSN + ";" + extraConnStr;
  SQLSMALLINT outConnStrLen   = 0;

  ret = SQLDriverConnect(hDbc,
                         nullptr,
                         (SQLCHAR*)inConnectionStr.c_str(),
                         static_cast<SQLSMALLINT>(inConnectionStr.size()),
                         nullptr,
                         0,
                         &outConnStrLen,
                         SQL_DRIVER_NOPROMPT);
  this->maybeReportConnectionError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLConnect";
}

void SQLDriverConnectFixture::TearDown() {
  // Disconnect from the data source
  if (hDbc) {
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
  }
  // Free environment handle
  if (hEnv) {
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
  }
}

void SQLDriverConnectFixture::maybeReportStatementError(SQLRETURN ret) {
  if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
    SQLCHAR sqlState[1024];
    SQLCHAR errorMsg[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT msgLength;
    SQLGetDiagRec(SQL_HANDLE_STMT,
                  this->hStmt,
                  1,
                  sqlState,
                  &nativeError,
                  errorMsg,
                  sizeof(errorMsg),
                  &msgLength);
    printf("Statement operation failed with error: %s\n", errorMsg);
  }
}

void SQLDriverConnectFixture::maybeReportConnectionError(SQLRETURN ret) {
  if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
    SQLCHAR sqlState[1024];
    SQLCHAR errorMsg[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT msgLength;
    SQLGetDiagRec(SQL_HANDLE_DBC,
                  this->hDbc,
                  1,
                  sqlState,
                  &nativeError,
                  errorMsg,
                  sizeof(errorMsg),
                  &msgLength);
    printf("Connection operation failed with error: %s\n", errorMsg);
  }
}
