#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#include "../constants.hpp"

class ConnectTest : public ::testing::Test {
  protected:
    SQLHENV hEnv   = nullptr;
    SQLHDBC hDbc   = nullptr;
    SQLHSTMT hStmt = nullptr;

    void SetUp() override {
      // PyODBC does this before allocating an environment handle.
      // We will do it as well for this test.
      SQLRETURN ret = SQLSetEnvAttr(nullptr,
                                    SQL_ATTR_CONNECTION_POOLING,
                                    (SQLPOINTER)SQL_CP_ONE_PER_HENV,
                                    4);

      // Allocate environment handle
      ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Set the ODBC version environment attribute
      ret = SQLSetEnvAttr(
          hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Allocate connection handle
      ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
      ASSERT_EQ(ret, SQL_SUCCESS);
    }

    void TearDown() override {
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
};

TEST_F(ConnectTest, ConnectWithSQLConnect) {

  // Connect to the data source
  std::string testDSN = TEST_DSN;
  SQLCHAR user[]      = "";
  SQLCHAR auth[]      = "";

  SQLRETURN ret;
  ret = SQLConnect(this->hDbc,
                   (SQLCHAR*)testDSN.c_str(),
                   static_cast<SQLSMALLINT>(testDSN.length()),
                   user,
                   0,
                   auth,
                   0);

  ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(ConnectTest, ConnectWithSQLDriverConnect) {
  // Connect to the data source
  std::string inConnectionStr = "DSN=" + TEST_DSN;
  SQLSMALLINT outConnStrLen   = 0;

  SQLRETURN ret;
  ret = SQLDriverConnect(this->hDbc,
                         nullptr,
                         (SQLCHAR*)(inConnectionStr.c_str()),
                         static_cast<SQLSMALLINT>(inConnectionStr.size()),
                         nullptr,
                         0,
                         &outConnStrLen,
                         SQL_DRIVER_NOPROMPT);

  ASSERT_EQ(ret, SQL_SUCCESS);
}
