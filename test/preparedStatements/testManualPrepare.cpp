#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

#include "../constants.hpp"
#include "../fixtures/sqlDriverConnectFixture.hpp"

class ManualPrepareTest : public SQLDriverConnectFixture {};

// Helper: SQLExecDirect a query and poll it to completion via SQLFetch.
// Returns the SQLExecDirect return code. If it succeeds, drains all
// rows so that subsequent statements on the same handle work correctly.
static SQLRETURN execDirectAndDrain(SQLHSTMT hStmt, const std::string& sql) {
  SQLRETURN ret = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
  if (ret != SQL_SUCCESS) {
    return ret;
  }
  // Drain rows to trigger polling / complete the query.
  while (SQLFetch(hStmt) == SQL_SUCCESS) {
  }
  return SQL_SUCCESS;
}

// Helper: SQLExecDirect a query, fetch one row, read a BIGINT from column 1.
static SQLRETURN execDirectAndReadBigint(SQLHSTMT hStmt,
                                         const std::string& sql,
                                         SQLBIGINT& result) {
  SQLRETURN ret = SQLExecDirect(hStmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
  if (ret != SQL_SUCCESS) {
    return ret;
  }
  ret = SQLFetch(hStmt);
  if (ret != SQL_SUCCESS) {
    return ret;
  }
  ret = SQLGetData(hStmt, 1, SQL_C_SBIGINT, &result, sizeof(result), NULL);
  // Drain any remaining rows.
  while (SQLFetch(hStmt) == SQL_SUCCESS) {
  }
  return ret;
}

TEST_F(ManualPrepareTest, TestManualPrepareExecuteAndDeallocate) {
  // Allocate a statement handle that will be reused for every step.
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);

  // 1. PREPARE add_one
  ret = execDirectAndDrain(hStmt, R"(PREPARE add_one FROM SELECT ? + 1)");
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to PREPARE add_one";

  // 2. PREPARE add_two
  ret = execDirectAndDrain(hStmt, R"(PREPARE add_two FROM SELECT ? + 2)");
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to PREPARE add_two";

  // 3. EXECUTE add_one USING 42 → expect 43
  SQLBIGINT resultOne = 0;
  ret = execDirectAndReadBigint(hStmt, "EXECUTE add_one USING 42", resultOne);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to EXECUTE add_one";
  ASSERT_EQ(resultOne, 43);

  // 4. EXECUTE add_two USING 42 → expect 44
  SQLBIGINT resultTwo = 0;
  ret = execDirectAndReadBigint(hStmt, "EXECUTE add_two USING 42", resultTwo);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to EXECUTE add_two";
  ASSERT_EQ(resultTwo, 44);

  // 5. DEALLOCATE PREPARE add_one
  ret = execDirectAndDrain(hStmt, "DEALLOCATE PREPARE add_one");
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to DEALLOCATE PREPARE add_one";

  // 6. EXECUTE add_one USING 42 → should fail (statement not found)
  ret = SQLExecDirect(hStmt, (SQLCHAR*)"EXECUTE add_one USING 42", SQL_NTS);
  // The POST itself may succeed (Trino accepts the query), but
  // the error surfaces during polling. Either way, we should not
  // get a valid result.
  if (ret == SQL_SUCCESS) {
    ret = SQLFetch(hStmt);
    ASSERT_NE(ret, SQL_SUCCESS)
        << "EXECUTE add_one should fail after deallocation";
  }

  // 7. EXECUTE add_two USING 42 → should still work (expect 44)
  SQLBIGINT resultTwoAgain = 0;
  ret                      = execDirectAndReadBigint(
      hStmt, "EXECUTE add_two USING 42", resultTwoAgain);
  ASSERT_EQ(ret, SQL_SUCCESS) << "EXECUTE add_two should still work";
  ASSERT_EQ(resultTwoAgain, 44);

  // 8. DEALLOCATE PREPARE add_two
  ret = execDirectAndDrain(hStmt, "DEALLOCATE PREPARE add_two");
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to DEALLOCATE PREPARE add_two";

  // 9. EXECUTE add_two USING 42 → should fail (statement not found)
  ret = SQLExecDirect(hStmt, (SQLCHAR*)"EXECUTE add_two USING 42", SQL_NTS);
  if (ret == SQL_SUCCESS) {
    ret = SQLFetch(hStmt);
    ASSERT_NE(ret, SQL_SUCCESS)
        << "EXECUTE add_two should fail after deallocation";
  }

  // Free statement handle
  ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS);
  hStmt = nullptr;
}
