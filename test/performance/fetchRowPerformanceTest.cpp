#include <windows.h>

#include <chrono>
#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class FetchRowPerformanceTest : public SQLDriverConnectFixture {
  protected:
    void SetUp() override {
      // Set up with "Warn" log level instead of the default, so the
      // logging takes less time and the performance test is more reflective
      // of reality.
      return SQLDriverConnectFixture::SetUp("LogLevel=Warn;");
    }

    void executeAndCountRows(const std::string& query, int expectedNumRows) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS)
          << "Failed to SQLAllocHandle a statement handle";

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to SQLExeceDirect";

      // Iterate the rows
      int rowCount = 0;
      while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
        ASSERT_EQ(ret, SQL_SUCCESS) << "Fetch unsuccessful";
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

TEST_F(FetchRowPerformanceTest, Select1KRows) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndCountRows(
      "SELECT custkey FROM tpch.tiny.customer WHERE custkey <= 1000", 1000);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under one second on most PCs
  ASSERT_LT(duration, 1000) << "Test ran too slowly...";
}

TEST_F(FetchRowPerformanceTest, Select100KRows) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndCountRows(
      "SELECT name FROM tpch.sf1.customer WHERE custkey <= 100000", 100000);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under ten seconds on most PCs
  ASSERT_LT(duration, 10000) << "Test ran too slowly...";
}

TEST_F(FetchRowPerformanceTest, Select1MRows) {
  auto begin = std::chrono::high_resolution_clock::now();
  executeAndCountRows(
      "SELECT name FROM tpch.sf100.customer WHERE custkey <= 1000000", 1000000);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  // This should complete in under ten seconds on most PCs
  ASSERT_LT(duration, 10000) << "Test ran too slowly...";
}
