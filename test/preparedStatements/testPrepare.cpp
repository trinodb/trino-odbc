#include <windows.h>

#include <gtest/gtest.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class SQLPrepareTest : public SQLDriverConnectFixture {};

TEST_F(SQLPrepareTest, TestPreparedExecutionWithoutParameters) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Prepare a SQL statement
  std::string query = R"SQL(
      SELECT *
      FROM tpch.sf1.customer
      WHERE custkey = 42
  )SQL";
  ret               = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);


  // Execute the statement.
  ret = SQLExecute(hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Fetch results for custkey 1
  ret = SQLFetch(hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Get a cell of data from the result.
  SQLBIGINT resultOne;
  ret =
      SQLGetData(hStmt, 1, SQL_C_SBIGINT, &resultOne, sizeof(resultOne), NULL);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // The result is the customer key, we're just selecting out
  // the same key we used for querying. It's a simple test, but
  // it verifies the thing we want to verify here.
  ASSERT_EQ(resultOne, 42);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
}


TEST_F(SQLPrepareTest, TestPreparedExecutionWithInputParameters) {
  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Prepare a SQL statement. For the purposes of this test, we're adding
  // a second parameter that's a string instead of an integer.
  // This exercises the code paths for multiple parameters and multiple
  // types of parameters instead of just one.
  std::string query = R"SQL(
      SELECT *
      FROM tpch.sf1.customer
      WHERE true
         AND custkey = ?
         AND name LIKE ?
  )SQL";
  ret               = SQLPrepare(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Bind the first parameter
  SQLINTEGER custkeyParam;

  ret = SQLBindParameter(hStmt,
                         1,
                         SQL_PARAM_INPUT,
                         SQL_C_SLONG,
                         SQL_INTEGER,
                         0,
                         0,
                         &custkeyParam,
                         0,
                         NULL);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Bind the second parameter
  SQLCHAR nameLikeParam[16] = {'\0'};

  ret = SQLBindParameter(hStmt,
                         2,
                         SQL_PARAM_INPUT,
                         SQL_C_CHAR,
                         SQL_CHAR,
                         0,
                         0,
                         nameLikeParam,
                         sizeof(nameLikeParam),
                         NULL);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);


  // Execute the statement for custkey 1. The
  // two keys should be the same.
  custkeyParam           = 42;
  std::string likeString = "Customer#%";
  strncpy_s(reinterpret_cast<char*>(nameLikeParam),
            sizeof(nameLikeParam),
            likeString.c_str(),
            likeString.size());
  ret = SQLExecute(hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Fetch results for custkey 1
  ret = SQLFetch(hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Get some data from the result.
  SQLBIGINT resultOne = 0;
  ret =
      SQLGetData(hStmt, 1, SQL_C_SBIGINT, &resultOne, sizeof(resultOne), NULL);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // The result is the customer key, we're just selecting out
  // the same key we used for querying. It's a simple test, but
  // it verifies the thing we want to verify here.
  ASSERT_EQ(custkeyParam, resultOne);

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  maybeReportStatementError(ret);
  ASSERT_EQ(ret, SQL_SUCCESS);
}
