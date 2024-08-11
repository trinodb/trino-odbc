#include "gtest/gtest.h"
#include <string>

#include "../../../src/util/cryptUtils.hpp"

TEST(CryptUtilsTest, RoundTripText) {
  std::string secret    = "thisIsASecret";
  std::string encrypted = encryptString(secret);
  std::string decrypted = decryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}

TEST(CryptUtilsTest, RoundTripEmptyString) {
  std::string secret    = "";
  std::string encrypted = encryptString(secret);
  std::string decrypted = decryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}
