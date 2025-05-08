#include <windows.h>

#include <gtest/gtest.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class SQLDescribColTest : public SQLDriverConnectFixture {};

void setupTpchCustomerQuery(SQLHSTMT hStmt) {
  std::string query = R"SQL(
      SELECT *
      FROM tpch.sf1.customer
  )SQL";
  SQLRETURN ret     = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
  ASSERT_EQ(ret, SQL_SUCCESS);
}

void setupTpchTinyintQuery(SQLHSTMT hStmt) {
  /*
  tpch doesn't contain any TINYINTs, so we can create one
  manually on this very small nation table to enable testing
  that type
  */
  std::string query = R"SQL(
      SELECT
        CAST(nationkey AS TINYINT) AS nationkey,
        name,
        regionkey,
        comment
      FROM tpch.sf1.nation
  )SQL";
  SQLRETURN ret     = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
  ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(SQLDescribColTest, TestDescribeBigintCol) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Run the query
  setupTpchCustomerQuery(hStmt);

  // Prepare buffers for SQLDescribeCol
  SQLCHAR colName[128];
  SQLSMALLINT nameLen       = 0;
  SQLSMALLINT dataType      = 0;
  SQLULEN colSize           = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable      = 0;

  // Describe the first column (custkey)
  ret = SQLDescribeCol(hStmt,
                       1, // Column number (1-based)
                       colName,
                       sizeof(colName),
                       &nameLen,
                       &dataType,
                       &colSize,
                       &decimalDigits,
                       &nullable);
  ASSERT_EQ(ret, SQL_SUCCESS);

  EXPECT_STREQ((const char*)colName, "custkey");
  EXPECT_EQ(dataType, SQL_BIGINT);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(SQLDescribColTest, TestDescribeVarcharCol) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Run the query
  setupTpchCustomerQuery(hStmt);

  // Prepare buffers for SQLDescribeCol
  SQLCHAR colName[128];
  SQLSMALLINT nameLen       = 0;
  SQLSMALLINT dataType      = 0;
  SQLULEN colSize           = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable      = 0;

  // Describe the second column (name)
  ret = SQLDescribeCol(hStmt,
                       2, // Column number (1-based)
                       colName,
                       sizeof(colName),
                       &nameLen,
                       &dataType,
                       &colSize,
                       &decimalDigits,
                       &nullable);
  ASSERT_EQ(ret, SQL_SUCCESS);

  EXPECT_STREQ((const char*)colName, "name");
  EXPECT_EQ(dataType, SQL_VARCHAR);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
}


TEST_F(SQLDescribColTest, TestDescribeDoubleColumn) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Run the query
  setupTpchCustomerQuery(hStmt);

  // Prepare buffers for SQLDescribeCol
  SQLCHAR colName[128];
  SQLSMALLINT nameLen       = 0;
  SQLSMALLINT dataType      = 0;
  SQLULEN colSize           = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable      = 0;

  // Describe the sixth column (acctbal)
  ret = SQLDescribeCol(hStmt,
                       6, // Column number (1-based)
                       colName,
                       sizeof(colName),
                       &nameLen,
                       &dataType,
                       &colSize,
                       &decimalDigits,
                       &nullable);
  ASSERT_EQ(ret, SQL_SUCCESS);

  EXPECT_STREQ((const char*)colName, "acctbal");
  EXPECT_EQ(dataType, SQL_DOUBLE);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
}

TEST_F(SQLDescribColTest, TestDescribeTinyintColumn) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Run the query
  setupTpchTinyintQuery(hStmt);

  // Prepare buffers for SQLDescribeCol
  SQLCHAR colName[128];
  SQLSMALLINT nameLen       = 0;
  SQLSMALLINT dataType      = 0;
  SQLULEN colSize           = 0;
  SQLSMALLINT decimalDigits = 0;
  SQLSMALLINT nullable      = 0;

  // Describe the first column (nationkey)
  ret = SQLDescribeCol(hStmt,
                       1, // Column number (1-based)
                       colName,
                       sizeof(colName),
                       &nameLen,
                       &dataType,
                       &colSize,
                       &decimalDigits,
                       &nullable);
  ASSERT_EQ(ret, SQL_SUCCESS);

  EXPECT_STREQ((const char*)colName, "nationkey");
  EXPECT_EQ(dataType, SQL_TINYINT);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
}
