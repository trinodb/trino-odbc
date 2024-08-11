#include <windows.h>

#include <chrono>
#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../../src/driver/constants/statementAttrs.hpp"
#include "../../src/trinoAPIWrapper/trinoQuery.hpp"
#include "../constants.hpp"
#include "../fixtures/sqlDriverConnectFixture.hpp"

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <typename T>
concept StructType = std::is_class_v<T> && !std::is_union_v<T>;

class FetchGetDataPerformanceTest : public SQLDriverConnectFixture {
  protected:
    void SetUp() override {
      // Set up with "Warn" log level instead of the default, so the
      // logging takes less time and the performance test is more reflective
      // of reality.
      return SQLDriverConnectFixture::SetUp("LogLevel=Warn;");
    }

    template <Numeric T>
    void executeAndValidateQuery(const std::string& query,
                                 int expectedNumRows,
                                 SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS)
          << "Failed to SQLAllocHandle a statement handle";

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLExecDirect";

      // Set the polling mode for SQLFetch
      TrinoQueryPollMode pollMode = ToCompletion;
      ret                         = SQLSetStmtAttr(
          hStmt, SQL_ATTR_DEFAULT_FETCH_POLL_MODE, &pollMode, SQL_IS_INTEGER);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to set SQLFetch poll mode";

      // Iterate the rows
      int rowCount = 0;
      ret          = SQLFetch(hStmt);
      while (ret != SQL_NO_DATA) {
        rowCount++;
        ASSERT_EQ(ret, SQL_SUCCESS) << "Fetch unsuccessful";

        // GetData to a numeric variable
        T result            = 0;
        SQLLEN indicator    = 0;
        SQLLEN bufferLength = -1;
        ret = SQLGetData(hStmt, 1, cType, &result, bufferLength, &indicator);
        ASSERT_EQ(ret, SQL_SUCCESS);
        ASSERT_TRUE(result >= 0) << "Impossible result value: " << result;

        ret = SQLFetch(hStmt);
      }

      // Check that we got the correct number of rows.
      ASSERT_EQ(rowCount, expectedNumRows)
          << "Wrong number of rows in result set";

      // Free statement handle
      SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    void executeAndValidateQueryChars(const std::string& query,
                                      int expectedNumRows,
                                      SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS)
          << "Failed to SQLAllocHandle a statement handle";

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLExecDirect query";

      // Set the polling mode for SQLFetch
      TrinoQueryPollMode pollMode = ToCompletion;
      ret                         = SQLSetStmtAttr(
          hStmt, SQL_ATTR_DEFAULT_FETCH_POLL_MODE, &pollMode, SQL_IS_INTEGER);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to set SQLFetch poll mode";

      // Iterate the rows
      int rowCount = 0;
      while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
        ASSERT_EQ(ret, SQL_SUCCESS) << "Fetch unsuccessful";
        rowCount++;

        // GetData to a char array variable
        char result[128];
        int bufferLength = sizeof(result);
        std::fill_n(result, bufferLength, '\0');
        SQLLEN indicator = 0;
        ret = SQLGetData(hStmt, 1, cType, &result, bufferLength, &indicator);
        ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLGetData";
        ASSERT_TRUE(strlen(result) > 0) << "Impossible result value";
      }

      // Check that we got the correct number of rows.
      ASSERT_EQ(rowCount, expectedNumRows)
          << "Wrong number of rows in result set";

      // Free statement handle
      SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
};

// Compiling in debug mode makes these performance tests significantly
// slower. We should only trust them for release mode tests.

TEST_F(FetchGetDataPerformanceTest, Select1KIntegers) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndValidateQuery<SQLBIGINT>(
      "SELECT custkey FROM tpch.tiny.customer WHERE custkey <= 1000",
      1000,
      SQL_C_SBIGINT);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under one second on most PCs
  ASSERT_LT(duration, 1000) << "Test ran too slowly...";
}

TEST_F(FetchGetDataPerformanceTest, Select1KVarchars) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndValidateQueryChars(
      "SELECT name FROM tpch.tiny.customer WHERE custkey <= 1000",
      1000,
      SQL_C_CHAR);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under one second on most PCs
  ASSERT_LT(duration, 1000) << "Test ran too slowly...";
}

TEST_F(FetchGetDataPerformanceTest, Select100KIntegers) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndValidateQuery<SQLBIGINT>(
      "SELECT custkey FROM tpch.sf100.customer WHERE custkey <= 100000",
      100000,
      SQL_C_SBIGINT);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under 10 seconds on most PCs
  ASSERT_LT(duration, 10000) << "Test ran too slowly...";
}

TEST_F(FetchGetDataPerformanceTest, Select100KIntegersFromWideTable) {
  auto begin        = std::chrono::high_resolution_clock::now();
  std::string query = R"SQL(
  -- 100 Columns of data.
  select
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey,
    custkey, custkey, custkey, custkey, custkey
  from
      tpch.sf100.customer
  where
      custkey <= 100000
  )SQL";
  executeAndValidateQuery<SQLBIGINT>(query, 100000, SQL_C_SBIGINT);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under 10 seconds on most PCs
  ASSERT_LT(duration, 10000) << "Test ran too slowly...";
}

TEST_F(FetchGetDataPerformanceTest, Select100KVarchars) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndValidateQueryChars(
      "SELECT name FROM tpch.sf100.customer WHERE custkey <= 100000",
      100000,
      SQL_C_CHAR);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under 10 seconds on most PCs
  ASSERT_LT(duration, 10000) << "Test ran too slowly...";
}
