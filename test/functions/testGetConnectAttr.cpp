#include <windows.h>

#include <sql.h>
#include <sqlext.h>

#include <gtest/gtest.h>

#include "../fixtures/sqlDriverConnectFixture.hpp"

class GetConnectAttrTest : public SQLDriverConnectFixture {};

TEST_F(GetConnectAttrTest, GetCurrentCatalog) {
  // Excel does this when trying to use the driver with
  // the "Get Data"/PowerQuery stuff.
  // It calls it twice, presumably to get the array size
  // first, and then again to put the string of that length
  // into a dynamically allocated array. We'll assume that's
  // right and copy that logic into this test.
  SQLINTEGER strLen = 0;

  // The first call is just to get the string length.
  SQLRETURN ret = SQLGetConnectAttr(
      this->hDbc, SQL_ATTR_CURRENT_CATALOG, nullptr, 0, &strLen);

  ASSERT_EQ(ret, SQL_SUCCESS);

  // Leave room for the null-termination character.
  unsigned char* buf = new unsigned char[strLen + 1]();

  std::fill_n(buf, strLen, '\0');

  ret = SQLGetConnectAttr(
      this->hDbc, SQL_ATTR_CURRENT_CATALOG, buf, strLen + 1, &strLen);

  ASSERT_EQ(ret, SQL_SUCCESS);

  // We can use the returned length to construct a proper string
  // from the char array to do this comparison.
  std::string currentCatalog(buf, buf + strLen);
  ASSERT_EQ(currentCatalog, std::string("system"));

  // Best clean up that dynamically allocated memory.
  delete[] buf;
}
