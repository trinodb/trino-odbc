#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class ColumnsTest : public SQLDriverConnectFixture {};

TEST_F(ColumnsTest, GetColumnsForBigint) {
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, this->hDbc, &hStmt);

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to allocate statement handle";

  std::string catalog = "tpch";
  std::string schema  = "sf1";
  std::string table   = "customer";
  std::string column  = "custkey";

  ret = SQLColumns(hStmt,
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(catalog.c_str())),
                   static_cast<SQLSMALLINT>(catalog.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(schema.c_str())),
                   static_cast<SQLSMALLINT>(schema.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(table.c_str())),
                   static_cast<SQLSMALLINT>(table.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(column.c_str())),
                   static_cast<SQLSMALLINT>(column.size()));

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to execute SQLColumns";

  // Run a fetch, since the results of this are returned as a result set.
  // There should be only one row.
  ret = SQLFetch(hStmt);
  ASSERT_TRUE(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
      << "No row returned from SQLColumns";

  SQLCHAR colName[256]     = {0};
  SQLCHAR typeName[256]    = {0};
  SQLSMALLINT dataType     = 0;
  SQLINTEGER colSize       = 0;
  SQLINTEGER decimalDigits = 0;

  // Column 4: COLUMN_NAME
  SQLGetData(hStmt, 4, SQL_C_CHAR, colName, sizeof(colName), NULL);
  // Column 5: DATA_TYPE
  SQLGetData(hStmt, 5, SQL_C_SSHORT, &dataType, 0, NULL);
  // Column 6: TYPE_NAME
  SQLGetData(hStmt, 6, SQL_C_CHAR, typeName, sizeof(typeName), NULL);
  // Column 7: COLUMN_SIZE
  SQLGetData(hStmt, 7, SQL_C_SLONG, &colSize, 0, NULL);
  // Column 9: DECIMAL_DIGITS
  SQLGetData(hStmt, 9, SQL_C_SLONG, &decimalDigits, 0, NULL);


  // Assert column name and type
  EXPECT_STREQ(reinterpret_cast<const char*>(colName), "custkey");
  EXPECT_STREQ(reinterpret_cast<const char*>(typeName), "bigint");
  EXPECT_EQ(dataType, SQL_BIGINT);
  EXPECT_EQ(colSize, 19);
  // Notably, this value differs from the value returned by
  // system.jdbc.columns. JDBC reports integral numeric types
  // as having NULL decimalDigits, but the ODBC spec says
  // it should have zero decimal digits. This driver implements
  // the ODBC behavior via some extra CASE statements. We test
  // that it works here.
  EXPECT_EQ(decimalDigits, 0);

  // Should be only one row
  ret = SQLFetch(hStmt);
  EXPECT_EQ(ret, SQL_NO_DATA);

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}


TEST_F(ColumnsTest, GetColumnsForTimestampWithTimeZone) {
  /*
  Timestamp with time zone is unique because Trino's system.jdbc.columns
  table returns values for this with a type code of 2014, which is
  not standard SQL. It's probably a proprietary extension that's understood
  only by their own JDBC driver.

  This test verifies that it gets reported as a SQL_TYPE_TIMESTAMP instead,
  which is the closest we can get with the vanilla ODBC 3.5 spec.

  Notably, this is a TIMESTAMP(3) column, so it has "DECIMAL_DIGITS=3"
  */
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, this->hDbc, &hStmt);

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to allocate statement handle";

  std::string catalog = "system";
  std::string schema  = "runtime";
  std::string table   = "queries";
  std::string column  = "created";

  ret = SQLColumns(hStmt,
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(catalog.c_str())),
                   static_cast<SQLSMALLINT>(catalog.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(schema.c_str())),
                   static_cast<SQLSMALLINT>(schema.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(table.c_str())),
                   static_cast<SQLSMALLINT>(table.size()),
                   const_cast<unsigned char*>(
                       reinterpret_cast<const unsigned char*>(column.c_str())),
                   static_cast<SQLSMALLINT>(column.size()));

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to execute SQLColumns";

  // Run a fetch, since the results of this are returned as a result set.
  // There should be only one row.
  ret = SQLFetch(hStmt);
  ASSERT_TRUE(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
      << "No row returned from SQLColumns";

  SQLCHAR colName[256]     = {0};
  SQLCHAR typeName[256]    = {0};
  SQLSMALLINT dataType     = 0;
  SQLINTEGER colSize       = 0;
  SQLINTEGER decimalDigits = 0;


  // Column 4: COLUMN_NAME
  SQLGetData(hStmt, 4, SQL_C_CHAR, colName, sizeof(colName), NULL);
  // Column 5: DATA_TYPE
  SQLGetData(hStmt, 5, SQL_C_SSHORT, &dataType, 0, NULL);
  // Column 6: TYPE_NAME
  SQLGetData(hStmt, 6, SQL_C_CHAR, typeName, sizeof(typeName), NULL);
  // Column 7: COLUMN_SIZE
  SQLGetData(hStmt, 7, SQL_C_SLONG, &colSize, 0, NULL);
  // Column 9: DECIMAL_DIGITS
  SQLGetData(hStmt, 9, SQL_C_SLONG, &decimalDigits, 0, NULL);


  // Assert column name and type
  EXPECT_STREQ(reinterpret_cast<const char*>(colName), "created");
  EXPECT_STREQ(reinterpret_cast<const char*>(typeName), "timestamp");
  EXPECT_EQ(dataType, SQL_TYPE_TIMESTAMP);
  EXPECT_EQ(colSize, sizeof(SQL_TIMESTAMP_STRUCT));
  // This column is a TIMESTAMP(3) WITH TIME ZONE, so we can test that "3".
  EXPECT_EQ(decimalDigits, 3);

  // Should be only one row
  ret = SQLFetch(hStmt);
  EXPECT_EQ(ret, SQL_NO_DATA);

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
