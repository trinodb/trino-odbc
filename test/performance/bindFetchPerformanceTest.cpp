#include <windows.h>

#include <chrono>
#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class FetchBindPerformanceTest : public SQLDriverConnectFixture {
  protected:
    void SetUp() override {
      // Set up with "Warn" log level instead of the default, so the
      // logging takes less time and the performance test is more reflective
      // of reality.
      return SQLDriverConnectFixture::SetUp("LogLevel=Warn;");
    }

    template <typename T>
    void executeAndValidateQuery(const std::string& query,
                                 int expectedNumRows,
                                 SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS)
          << "Failed to SQLAllocHandle a statement handle";

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLExeceDirect";

      // Bind the column to a variable
      T result         = -1;
      SQLLEN indicator = 0;
      int bufferLength = -1; // For fixed length types, this is ignored.
      ret = SQLBindCol(hStmt, 1, cType, &result, bufferLength, &indicator);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to bind column.";

      // Iterate the rows
      int rowCount = 0;
      while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
        ASSERT_EQ(ret, SQL_SUCCESS) << "Fetch unsuccessful";
        ASSERT_TRUE(result > -1) << "Impossible result value: " << result;
        ++rowCount;
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

      // Bind the column to an integer variable
      char result[128];
      int bufferLength = sizeof(result);
      std::fill_n(result, bufferLength, '\0');
      SQLLEN indicator = 0;
      ret = SQLBindCol(hStmt, 1, cType, &result, bufferLength, &indicator);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to bind column";

      // Iterate the rows
      int rowCount = 0;
      while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
        ASSERT_EQ(ret, SQL_SUCCESS) << "Fetch unsuccessful";
        ASSERT_TRUE(strlen(result) > 0) << "Impossible result value";
        ++rowCount;
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

TEST_F(FetchBindPerformanceTest, Select1KIntegers) {
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

TEST_F(FetchBindPerformanceTest, Select1KVarchars) {
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

TEST_F(FetchBindPerformanceTest, Select100KIntegers) {
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

TEST_F(FetchBindPerformanceTest, Select100KVarchars) {
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
