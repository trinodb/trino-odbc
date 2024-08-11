#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#include "../fixtures/sqlDriverConnectFixture.hpp"

/*
 Bringing in the statement handle implementation so we
 can inspect it.
 */
#include "../../src/driver/handles/statementHandle.hpp"
/*
 Bring in the TrinoQuery so we recognize that this
 test is a friend class.
 */
#include "../../src/trinoAPIWrapper/trinoQuery.hpp"
/*
 Bring in the definitions for driver-defined statement
 attributes.
*/
#include "../../src/driver/constants/statementAttrs.hpp"

class MemoryReclamationTest : public SQLDriverConnectFixture {
  protected:
    void SetUp() override {
      // Set up with "Warn" log level instead of the default, so the
      // logging takes less time and the performance test is more reflective
      // of reality.
      return SQLDriverConnectFixture::SetUp("LogLevel=Warn");
    }

    /*
    This works because this test fixture is a friend class of
    TrinoQuery. It's not pretty, but I think it's worth it
    to have tests of the memory behavior.
    */
    size_t CheckTrinoQueryInternalRowCount(TrinoQuery* trinoQuery) {
      return trinoQuery->dataJson.size();
    }
};

TEST_F(MemoryReclamationTest, TestMemoryReclaimedDuringQuery) {
  // This query returns 25,000 rows, which should trigger pagination.
  // and, if this driver is behaving, memory reclamation.
  std::string big_query = R"SQL(
    SELECT *
    FROM tpch.sf1.customer
    LIMIT 25000
  )SQL";

  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Because we want to monitor the inner workings of the
  // TrinoQuery object's memory, we will need to cast the
  // statement handle to a pointer to the actual statement
  // implementation. Clients of the ODBC driver would never
  // do this, but we'll do it in this unit test. This
  // test is also a friend class to TrinoQuery, so we can
  // inspect its private internals to infer if memory
  // is being reclaimed.

  Statement* actualStatementPtr = nullptr;
  SQLINTEGER stringLength       = 0;
  ret                           = SQLGetStmtAttr(hStmt,
                       SQL_ATTR_RAW_STATEMENT_HANDLE,
                       &actualStatementPtr,
                       SQL_IS_POINTER,
                       &stringLength);
  ASSERT_EQ(ret, SQL_SUCCESS);
  TrinoQuery* trinoQueryPtr = actualStatementPtr->trinoQuery;

  // Execute the query
  ret = SQLExecDirect(hStmt, (SQLCHAR*)big_query.c_str(), SQL_NTS);
  ASSERT_EQ(ret, SQL_SUCCESS);

  int timesCacheSizeChanged = 0;
  size_t lastNumRows        = 0;
  int nRows                 = 0;
  while (true) {
    // Fetch the next row of data.
    ret = SQLFetch(hStmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else {
      ASSERT_EQ(ret, SQL_SUCCESS);
    }

    size_t newNumRows = CheckTrinoQueryInternalRowCount(trinoQueryPtr);
    if (newNumRows != lastNumRows) {
      timesCacheSizeChanged++;
    }
    lastNumRows = newNumRows;

    // Get the data
    SQLSMALLINT cType   = SQL_C_SBIGINT;
    SQLBIGINT result    = 0;
    SQLLEN bufferLength = sizeof(result);
    SQLLEN indicator    = 0;
    ret = SQLGetData(hStmt, 1, cType, &result, bufferLength, &indicator);
    ASSERT_EQ(ret, SQL_SUCCESS);
    nRows++;
  }

  // One test is that the number of rows in the cache
  // should be less than the number of rows in the query.
  ASSERT_LT(lastNumRows, 25000)
      << "Cache contained all rows, no memory reclaimed";

  // The big test is that we should still count the correct number
  // of rows, no off-by-one errors or anything.
  ASSERT_EQ(nRows, 25000)
      << "Wrong number of rows returned during paginated query";

  // We also need to observe the row cache size change multiple
  // times to be confident that the checkpointing/reclamation
  // logic is doing what it is supposed to.
  ASSERT_GE(timesCacheSizeChanged, 4)
      << "Row cache did not change during memory reclamation";

  // Free statement handle
  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

TEST_F(MemoryReclamationTest, TestCorrectAnswerWithMemoryReclamation) {
  // Memory reclamation is tricky, so let's test that we're reading
  // the correct set of rows with a query that has a well-defined answer.
  // The sum of nation keys for the first 25,000 customers is exactly
  // 300,022, confirmed with an aggregate query in DBeaver with Trino's
  // JDBC driver.
  int SUM_OF_NATION_KEYS = 300022;
  std::string big_query =
      R"SQL(
          SELECT nationkey
          FROM (
              SELECT *
              FROM tpch.sf1.customer
              ORDER BY custkey
              LIMIT 25000
          )
        )SQL";

  // Allocate statement handle
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // Execute the query
  ret = SQLExecDirect(hStmt, (SQLCHAR*)big_query.c_str(), SQL_NTS);
  ASSERT_EQ(ret, SQL_SUCCESS);

  int nationKeyRunningSum = 0;
  int nRows               = 0;
  while (true) {
    // Fetch the next row of data.
    ret = SQLFetch(hStmt);
    if (ret == SQL_NO_DATA) {
      break;
    } else {
      ASSERT_EQ(ret, SQL_SUCCESS);
    }

    // Get the data
    SQLSMALLINT cType   = SQL_INTEGER;
    SQLINTEGER result   = 0;
    SQLLEN bufferLength = sizeof(result);
    SQLLEN indicator    = 0;
    ret = SQLGetData(hStmt, 1, cType, &result, bufferLength, &indicator);
    ASSERT_EQ(ret, SQL_SUCCESS);
    nationKeyRunningSum += result;
    nRows++;
  }

  // First, a quick check that we actually pulled the correct
  // number of rows, no off-by-one errors or anything.
  ASSERT_EQ(nRows, 25000)
      << "Wrong number of rows returned during paginated query";

  // The most important test here is that the sum
  // we calculate exactly matches the correct sum,
  // meaning we get correct data even with memory
  // reclamation logic on big queries.
  ASSERT_EQ(nationKeyRunningSum, SUM_OF_NATION_KEYS)
      << "Incorrect sum of known-quantity query. Logic error?";

  // Free statement handle
  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
