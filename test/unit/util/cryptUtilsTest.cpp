#include "gtest/gtest.h"
#include <string>

#include "../../../src/util/cryptUtils.hpp"

TEST(CryptUtilsTest, RoundTripUserText) {
  std::string secret    = "thisIsASecret";
  std::string encrypted = userEncryptString(secret);
  std::string decrypted = userDecryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}

TEST(CryptUtilsTest, RoundTripUsesrEmptyString) {
  std::string secret    = "";
  std::string encrypted = userEncryptString(secret);
  std::string decrypted = userDecryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}

TEST(CryptUtilsTest, RoundTripSystemText) {
  std::string secret    = "thisIsASecret";
  std::string encrypted = systemEncryptString(secret);
  std::string decrypted = systemDecryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}

TEST(CryptUtilsTest, RoundTripSystemEmptyString) {
  std::string secret    = "";
  std::string encrypted = systemEncryptString(secret);
  std::string decrypted = systemDecryptString(encrypted);

  EXPECT_EQ(secret, decrypted);
}

TEST(CryptUtilsTest, UserAndSystemSecretsAreDifferent) {
  std::string secret          = "hello";
  std::string userEncrypted   = userEncryptString(secret);
  std::string systemEncrypted = systemEncryptString(secret);

  // The secrets should be encrypted differently depending on
  // whether they are user secrets or system secrets.
  EXPECT_NE(userEncrypted, systemEncrypted);

  std::string userDecrypted   = userDecryptString(userEncrypted);
  std::string systemDecrypted = systemDecryptString(systemEncrypted);

  // After decryption, the values should all be the same again
  // since we started from the same secret string.
  EXPECT_EQ(userDecrypted, systemDecrypted);
  EXPECT_EQ(userDecrypted, secret);
  EXPECT_EQ(systemDecrypted, secret);
}
