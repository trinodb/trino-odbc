#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"

#include "../fixtures/sqlDriverConnectFixture.hpp"

class GetTablesTest : public SQLDriverConnectFixture {};

TEST_F(GetTablesTest, GetCatalogs) {
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, this->hDbc, &hStmt);

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to allocate statement handle";

  std::string catalog   = SQL_ALL_CATALOGS;
  std::string schema    = "";
  std::string table     = "";
  std::string tableType = "";

  ret =
      SQLTables(hStmt,
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
                    reinterpret_cast<const unsigned char*>(tableType.c_str())),
                static_cast<SQLSMALLINT>(tableType.size()));

  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to execute SQLTables";

  SQLSMALLINT cType   = SQL_C_CHAR;
  char result[128]    = "";
  SQLLEN indicator    = 0;
  SQLLEN bufferLength = sizeof(result);
  ret = SQLBindCol(hStmt, 1, cType, result, bufferLength, &indicator);

  if (ret != SQL_SUCCESS) {
    SQLCHAR sqlState[6];
    std::fill_n(sqlState, 6, '\0');
    SQLINTEGER nativeError = 0;
    SQLCHAR errorMessage[1024];
    std::fill_n(errorMessage, 1024, '\0');
    SQLSMALLINT returnedLength = 0;

    SQLGetDiagRec(SQL_HANDLE_STMT,
                  hStmt,
                  1,
                  sqlState,
                  &nativeError,
                  errorMessage,
                  1024,
                  &returnedLength);
    ASSERT_TRUE(false) << errorMessage;
  }
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to bind column";

  // Fetch the data
  ret = SQLFetch(hStmt);
  ASSERT_EQ(ret, SQL_SUCCESS) << "Failed to fetch";

  // Obtaining the name of a catalog is enough to call this a success.
  std::string firstCatalogName(result, indicator);
  ASSERT_TRUE(not firstCatalogName.empty());
}
