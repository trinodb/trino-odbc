#include <windows.h>

#include <concepts>
#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>
#include <type_traits>

#include "../fixtures/sqlDriverConnectFixture.hpp"

template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <typename T>
concept StructType = std::is_class_v<T> && !std::is_union_v<T>;

class FetchBindTest : public SQLDriverConnectFixture {
  protected:
    template <Numeric T>
    void executeAndValidateQuery(const std::string& query,
                                 T expectedValue,
                                 SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Bind the column to an integer variable
      T result            = 0;
      SQLLEN indicator    = 0;
      SQLLEN bufferLength = -1; // Ignored for integer types.
      ret = SQLBindCol(hStmt, 1, cType, &result, bufferLength, &indicator);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Fetch the data
      ret = SQLFetch(hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Validate the result
      ASSERT_EQ(result, expectedValue);

      // Check that there are no more rows
      ret = SQLFetch(hStmt);
      ASSERT_EQ(ret, SQL_NO_DATA);

      // Free statement handle
      SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    void executeAndValidateQuery(const std::string& query,
                                 const std::string expectedValue,
                                 const SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Bind the column to an integer variable
      char result[127]    = {0};
      SQLLEN indicator    = 0;
      SQLLEN bufferLength = sizeof(result);
      ret = SQLBindCol(hStmt, 1, cType, result, bufferLength, &indicator);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Fetch the data
      ret = SQLFetch(hStmt);
      ASSERT_EQ(ret, SQL_SUCCESS);

      // Validate the result
      std::string resultStr(result, indicator > 0 ? indicator : 0);
      ASSERT_EQ(resultStr, expectedValue);

      // Check that there are no more rows
      ret = SQLFetch(hStmt);
      ASSERT_EQ(ret, SQL_NO_DATA);

      // Free statement handle
      SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    template <StructType T>
    T executeAndValidateQuery(const std::string& query, SQLSMALLINT cType) {
      // Allocate statement handle
      SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
      EXPECT_EQ(ret, SQL_SUCCESS);

      // Execute the query
      ret = SQLExecDirect(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
      EXPECT_EQ(ret, SQL_SUCCESS);

      // Bind the column to a struct/class variable
      T result{};
      SQLLEN indicator    = 0;
      SQLLEN bufferLength = sizeof(result);
      ret = SQLBindCol(hStmt, 1, cType, &result, bufferLength, &indicator);
      EXPECT_EQ(ret, SQL_SUCCESS);

      // Fetch the data
      ret = SQLFetch(hStmt);
      EXPECT_EQ(ret, SQL_SUCCESS);

      // Check that there are no more rows
      ret = SQLFetch(hStmt);
      EXPECT_EQ(ret, SQL_NO_DATA);

      // Free statement handle
      SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
      return result;
    }
};

TEST_F(FetchBindTest, SelectTinyInt) {
  executeAndValidateQuery(
      "SELECT CAST(1 AS TINYINT)", (SQLCHAR)1, SQL_C_TINYINT);
}

TEST_F(FetchBindTest, SelectSignedTinyInt) {
  executeAndValidateQuery(
      "SELECT CAST(1 AS TINYINT)", (SQLCHAR)1, SQL_C_STINYINT);
}

TEST_F(FetchBindTest, SelectSmallInt) {
  executeAndValidateQuery(
      "SELECT CAST(1 AS SMALLINT)", (SQLSMALLINT)1, SQL_C_SHORT);
}

TEST_F(FetchBindTest, SelectInt) {
  executeAndValidateQuery("SELECT CAST(1 AS INT)", 1, SQL_C_LONG);
}

TEST_F(FetchBindTest, SelectBigInt) {
  executeAndValidateQuery(
      "SELECT CAST(1 AS BIGINT)", (SQLBIGINT)1, SQL_C_SBIGINT);
}

TEST_F(FetchBindTest, SelectBoolean) {
  executeAndValidateQuery("SELECT true", (SQLCHAR)1, SQL_C_BIT);
}

TEST_F(FetchBindTest, SelectReal) {
  executeAndValidateQuery("SELECT CAST(1 AS REAL)", (SQLREAL)1, SQL_C_FLOAT);
}

TEST_F(FetchBindTest, SelectDouble) {
  executeAndValidateQuery(
      "SELECT CAST(1 AS DOUBLE)", (SQLDOUBLE)1, SQL_C_DOUBLE);
}

TEST_F(FetchBindTest, SelectVarchar) {
  std::string expectedVarchar = "abc";
  executeAndValidateQuery("SELECT 'abc'", expectedVarchar, SQL_C_CHAR);
}

TEST_F(FetchBindTest, SelectUUIDVarchar) {
  std::string expectedUUID = "00000001-0002-0003-0004-000000000005";
  executeAndValidateQuery(
      "SELECT CAST('00000001-0002-0003-0004-000000000005' AS UUID)",
      expectedUUID,
      SQL_C_CHAR);
}

TEST_F(FetchBindTest, SelectGUID) {
  SQLGUID expectedGuid = {1, 2, 3, {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'}};

  // 41 is ASCII capital A, 42 is ASCII capital B, etc.
  SQLGUID res = executeAndValidateQuery<SQLGUID>(
      "SELECT CAST('00000001-0002-0003-4142-434445464748' AS UUID)",
      SQL_C_GUID);

  ASSERT_EQ(res.Data1, expectedGuid.Data1);
  ASSERT_EQ(res.Data2, expectedGuid.Data2);
  ASSERT_EQ(res.Data3, expectedGuid.Data3);
  for (auto i = 0; i < 8; i++) {
    ASSERT_EQ(res.Data4[i], expectedGuid.Data4[i]);
  }
}

TEST_F(FetchBindTest, SelectPositiveDecimal) {
  // The goal is to select the number 123.45. We'll
  // hand-craft a tagSQL_NUMERIC_STRUCT that should
  // contain that value.
  // This particular value has a documented "correct" encoding:
  // https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831

  SQLCHAR precision = 5;
  SQLSCHAR scale    = 2;
  SQLCHAR sign      = 1;
  SQLCHAR val[SQL_MAX_NUMERIC_LEN];
  std::fill_n(val, 16, '\0');
  val[0] = 0x39;
  val[1] = 0x30;

  // Turn this into a struct.
  tagSQL_NUMERIC_STRUCT expectedDecimal = {precision, scale, sign, {0}};
  std::memcpy(expectedDecimal.val, val, SQL_MAX_NUMERIC_LEN);

  tagSQL_NUMERIC_STRUCT res = executeAndValidateQuery<tagSQL_NUMERIC_STRUCT>(
      "SELECT CAST('123.45' AS DECIMAL(5, 2))", SQL_C_NUMERIC);

  ASSERT_EQ(expectedDecimal.precision, res.precision);
  ASSERT_EQ(expectedDecimal.scale, res.scale);
  ASSERT_EQ(expectedDecimal.sign, res.sign);
  for (auto i = 0; i < SQL_MAX_NUMERIC_LEN; i++) {
    ASSERT_EQ(expectedDecimal.val[i], res.val[i]);
  }
}

TEST_F(FetchBindTest, SelectNegativeDecimal) {
  // The goal is to select the number -9876.543210
  // from a DECIMAL(12, 6). We'll hand-craft a
  // tagSQL_NUMERIC_STRUCT that should contain that value.

  // In this scenario, we have more precision than is required for
  // the number we want to express here. It fits in 10, but we're
  // casting to a precision of 12.
  SQLCHAR precision = 12;
  SQLSCHAR scale    = 6;
  SQLCHAR sign      = 0;
  SQLCHAR val[SQL_MAX_NUMERIC_LEN];
  std::fill_n(val, 16, '\0');
  val[0] = 0xEA;
  val[1] = 0x16;
  val[2] = 0xB0;
  val[3] = 0x4C;
  val[4] = 0x02;

  // Turn this into a struct.
  tagSQL_NUMERIC_STRUCT expectedDecimal = {precision, scale, sign, {0}};
  std::memcpy(expectedDecimal.val, val, SQL_MAX_NUMERIC_LEN);

  tagSQL_NUMERIC_STRUCT res = executeAndValidateQuery<tagSQL_NUMERIC_STRUCT>(
      "SELECT CAST('-9876.543210' AS DECIMAL(12, 6))", SQL_C_NUMERIC);

  ASSERT_EQ(expectedDecimal.precision, res.precision);
  ASSERT_EQ(expectedDecimal.scale, res.scale);
  ASSERT_EQ(expectedDecimal.sign, res.sign);
  for (auto i = 0; i < SQL_MAX_NUMERIC_LEN; i++) {
    ASSERT_EQ(expectedDecimal.val[i], res.val[i]);
  }
}

TEST_F(FetchBindTest, SelectIntegralDecimal) {
  // The goal is to select the number 12345
  // from a DECIMAL(10, 0). This is unique, because it's a
  // decimal that would easily fit into an integer, and has
  // a scale of zero. We'll hand-craft a tagSQL_NUMERIC_STRUCT
  // that should contain that value.

  SQLCHAR precision = 10;
  SQLSCHAR scale    = 0;
  SQLCHAR sign      = 1;
  SQLCHAR val[SQL_MAX_NUMERIC_LEN];
  std::fill_n(val, 16, '\0');
  val[0] = 0x39;
  val[1] = 0x30;

  // Turn this into a struct.
  tagSQL_NUMERIC_STRUCT expectedDecimal = {precision, scale, sign, {0}};
  std::memcpy(expectedDecimal.val, val, SQL_MAX_NUMERIC_LEN);

  tagSQL_NUMERIC_STRUCT res = executeAndValidateQuery<tagSQL_NUMERIC_STRUCT>(
      "SELECT CAST('12345' AS DECIMAL(10, 0))", SQL_C_NUMERIC);

  ASSERT_EQ(expectedDecimal.precision, res.precision);
  ASSERT_EQ(expectedDecimal.scale, res.scale);
  ASSERT_EQ(expectedDecimal.sign, res.sign);
  for (auto i = 0; i < SQL_MAX_NUMERIC_LEN; i++) {
    ASSERT_EQ(expectedDecimal.val[i], res.val[i]);
  }
}

TEST_F(FetchBindTest, SelectDecimalVarchar) {
  // We'll let trino truncate some digits (with rounding!)
  // for this test.
  std::string expectedDecimal = "12.346";
  executeAndValidateQuery("SELECT CAST('12.3456789' AS DECIMAL(5, 3))",
                          expectedDecimal,
                          SQL_C_CHAR);
}

TEST_F(FetchBindTest, SelectDate) {
  SQLSMALLINT year   = 2024;
  SQLUSMALLINT month = 9;
  SQLUSMALLINT day   = 13;

  SQL_DATE_STRUCT expectedDate = {
      year,
      month,
      day,
  };

  SQL_DATE_STRUCT res = executeAndValidateQuery<SQL_DATE_STRUCT>(
      "SELECT date '2024-09-13'", SQL_C_DATE);

  ASSERT_EQ(res.year, expectedDate.year);
  ASSERT_EQ(res.month, expectedDate.month);
  ASSERT_EQ(res.day, expectedDate.day);
}

TEST_F(FetchBindTest, SelectTime) {
  SQLUSMALLINT hour   = 4;
  SQLUSMALLINT minute = 10;
  SQLUSMALLINT second = 42;

  SQL_TIME_STRUCT expectedTime = {hour, minute, second};

  SQL_TIME_STRUCT res = executeAndValidateQuery<SQL_TIME_STRUCT>(
      "SELECT time '4:10:42'", SQL_C_TIME);

  ASSERT_EQ(res.hour, expectedTime.hour);
  ASSERT_EQ(res.minute, expectedTime.minute);
  ASSERT_EQ(res.second, expectedTime.second);
}

TEST_F(FetchBindTest, SelectTimestamp) {
  SQLSMALLINT year     = 2024;
  SQLUSMALLINT month   = 9;
  SQLUSMALLINT day     = 13;
  SQLUSMALLINT hour    = 4;
  SQLUSMALLINT minute  = 10;
  SQLUSMALLINT second  = 42;
  SQLUINTEGER fraction = 123000000; // Billionths of a second.

  SQL_TIMESTAMP_STRUCT expectedTimestamp = {
      year, month, day, hour, minute, second, fraction};

  SQL_TIMESTAMP_STRUCT res = executeAndValidateQuery<SQL_TIMESTAMP_STRUCT>(
      "SELECT timestamp '2024-09-13 4:10:42.123'", SQL_C_TIMESTAMP);

  ASSERT_EQ(res.year, expectedTimestamp.year);
  ASSERT_EQ(res.month, expectedTimestamp.month);
  ASSERT_EQ(res.day, expectedTimestamp.day);
  ASSERT_EQ(res.hour, expectedTimestamp.hour);
  ASSERT_EQ(res.minute, expectedTimestamp.minute);
  ASSERT_EQ(res.second, expectedTimestamp.second);
  ASSERT_EQ(res.fraction, expectedTimestamp.fraction);
}
