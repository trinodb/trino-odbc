#include <windows.h>

#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>

#include "../constants.hpp"
#include "../fixtures/sqlDriverConnectFixture.hpp"

class GetInfoTest : public SQLDriverConnectFixture {};

TEST_F(GetInfoTest, GetODBCVersion) {
  unsigned char buf[16];
  // Put some nonsense into the char array as a test.
  std::fill_n(buf, 16, 'A');
  SQLSMALLINT bufferLen        = 16;
  SQLSMALLINT StrLen_or_IndPtr = 0;
  SQLRETURN ret                = SQLGetInfo(
      this->hDbc, SQL_DRIVER_ODBC_VER, buf, bufferLen, &StrLen_or_IndPtr);

  // The string should be 5 long. The spec says the string length
  // is returned _excluding_ the null-termination character.
  ASSERT_EQ(StrLen_or_IndPtr, 5);
  // We can use the returned length to construct a proper string
  // from the char array to do this comparison.
  std::string odbcVer(buf, buf + StrLen_or_IndPtr);
  ASSERT_EQ(odbcVer, std::string("03.80"));
}

TEST_F(GetInfoTest, GetTrinoDBMSName) {
  unsigned char buf[16];
  // Put some nonsense into the char array as a test.
  std::fill_n(buf, 16, 'A');
  SQLSMALLINT bufferLen        = 16;
  SQLSMALLINT StrLen_or_IndPtr = 0;
  SQLRETURN ret =
      SQLGetInfo(this->hDbc, SQL_DBMS_NAME, buf, bufferLen, &StrLen_or_IndPtr);

  // The string should be 5 long. The spec says the string length
  // is returned _excluding_ the null-termination character.
  ASSERT_EQ(StrLen_or_IndPtr, 5);

  // We can use the returned length to construct a proper string
  // from the char array to do this comparison.
  std::string dbmsName(buf, buf + StrLen_or_IndPtr);
  ASSERT_EQ(dbmsName, std::string("Trino"));
}

TEST_F(GetInfoTest, GetTrinoDBMSVersion) {
  unsigned char buf[16];
  // Put some nonsense into the char array as a test.
  std::fill_n(buf, 16, 'A');
  SQLSMALLINT bufferLen        = 16;
  SQLSMALLINT StrLen_or_IndPtr = 0;
  SQLRETURN ret =
      SQLGetInfo(this->hDbc, SQL_DBMS_VER, buf, bufferLen, &StrLen_or_IndPtr);

  // The format of the version string is `##.##.#### <optional version>`
  // Our implementation should return, for example, `00.00.0001 488`
  // where we have a product-specific version string suffix on the
  // version string. The actual Trino version doesn't fit in the
  // mandatory DBMS version syntax prescribed by ODBC.
  // The result is a string 14 characters long.
  ASSERT_EQ(StrLen_or_IndPtr, 14);

  // We can use the returned length to construct a proper string
  // from the char array to do this comparison.
  std::string dbmsVers(buf, buf + StrLen_or_IndPtr);
  std::string firstPart = dbmsVers.substr(0, 10);
  ASSERT_EQ(firstPart, std::string("00.00.0001"));

  // We can parse the server version and assert that it is a three-digit
  // integer. We don't want to assert/mandate a specific server version
  // though.
  std::string secondPart = dbmsVers.substr(11);
  int trinoServerVersion = std::atoi(secondPart.c_str());
  ASSERT_GE(trinoServerVersion, 100);
}
