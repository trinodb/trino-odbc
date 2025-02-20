#include <gtest/gtest.h>
#include <string>

#include "../../../src/util/stringTrim.hpp"

TEST(StringTrimTest, EmptyString) {
  std::string str = "";
  trim(str);
  EXPECT_EQ(str, "");
}

TEST(StringTrimTest, NoWhitespace) {
  std::string str = "foo";
  trim(str);
  EXPECT_EQ(str, "foo");
}

TEST(StringTrimTest, LeftWhitespace) {
  std::string str = "\t\r\n foo";
  trim(str);
  EXPECT_EQ(str, "foo");
}

TEST(StringTrimTest, RightWhitespace) {
  std::string str = "foo \t\r\n";
  trim(str);
  EXPECT_EQ(str, "foo");
}

TEST(StringTrimTest, BothWhitespace) {
  std::string str = "\t\r\n foo \t\r\n";
  trim(str);
  EXPECT_EQ(str, "foo");
}

TEST(StringTrimTest, AllWhitespace) {
  std::string str = "\t\r\n ";
  trim(str);
  EXPECT_EQ(str, "");
}

TEST(StringTrimTest, HighBytesNoAssert) {
  // Bytes > 127 are negative when stored in signed char.
  // std::isspace with a negative int (other than EOF) is undefined
  // behaviour and triggers a debug assertion on MSVC.
  std::string str = " \xC0\xC1hello\xFF ";
  trim(str);
  EXPECT_EQ(str, "\xC0\xC1hello\xFF");
}
