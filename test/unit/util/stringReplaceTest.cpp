#include <gtest/gtest.h>
#include <string>

#include "../../../src/util/stringReplace.hpp"

TEST(StringReplaceTest, EmptyString) {
  std::string str    = "";
  std::string result = replaceAll(str, "a", "b");
  EXPECT_EQ(str, "");
}

TEST(StringReplaceTest, NoOccurrences) {
  std::string str    = "hello world";
  std::string result = replaceAll(str, "x", "y");
  EXPECT_EQ(result, "hello world");
}

TEST(StringReplaceTest, SingleOccurrence) {
  std::string str    = "hello world";
  std::string result = replaceAll(str, "world", "there");
  EXPECT_EQ(result, "hello there");
}

TEST(StringReplaceTest, MultipleOccurrences) {
  std::string str    = "the cat sat on the mat";
  std::string result = replaceAll(str, "at", "it");
  EXPECT_EQ(result, "the cit sit on the mit");
}

TEST(StringReplaceTest, SingleQuoteToDoubleSingleQuote) {
  std::string str    = "O'Brien";
  std::string result = replaceAll(str, "'", "''");
  EXPECT_EQ(result, "O''Brien");
}
