#include "gtest/gtest.h"
#include <string>

#include "../../../src/util/valuePtrHelper.hpp"

TEST(ValuePtrHelperTest, CanWriteNullTermStringToPtr) {
  // Setup
  std::string s   = "hello";
  char buffer[20] = {};
  short len       = 0;

  // Test
  writeNullTermStringToPtr(buffer, s, &len);


  // Assert
  EXPECT_STREQ(s.c_str(), buffer);
  EXPECT_EQ(len, s.length());
}

TEST(ValuePtrHelperTest, CanWriteEmptyStringToPtr) {
  // Setup
  std::string s  = "";
  char buffer[5] = {};
  short len      = 0;

  // Test
  writeNullTermStringToPtr(buffer, s, &len);

  // Assert
  EXPECT_STREQ(s.c_str(), buffer);
  EXPECT_EQ(len, 0);
}

TEST(ValuePtrHelperTest, NullCharInCorrectPosition) {
  /*
  Setup

  The buffer is actually 8 characters, but we're going to tell
  the function that it's 4 characters. Then we can look at the
  resulting buffer to confirm that it has the null terminator
  in the correct place (and no extras that would have
  overflowed the buffer).

  Buffer: "bbbbbbbb"
  String: "aaaa"
  Length: 4
  Expected Result: "aaaa\0bbb"
  */
  std::string s  = "aaaa";
  char buffer[9] = {"bbbbbbbb"};
  short len      = 4;

  // Test
  writeNullTermStringToPtr(buffer, s, &len);

  // Assert
  EXPECT_STREQ(s.c_str(), buffer); // Correct data
  EXPECT_EQ(len, s.length());      // Correct length
  EXPECT_EQ(buffer[4], '\0');      // Check for null terminator
  EXPECT_EQ(buffer[5], 'b');       // Check for correct padding
  EXPECT_EQ(buffer[6], 'b');       // Check for correct padding
  EXPECT_EQ(buffer[7], 'b');       // Check for correct padding
  EXPECT_EQ(buffer[8], '\0');      // Check for original null terminator.
}
