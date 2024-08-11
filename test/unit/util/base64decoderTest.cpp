#include "gtest/gtest.h"
#include <string>

#include "../../../src/util/b64decoder.hpp"

TEST(Base64urlDecoderTest, NoChars) {
  std::string encodedA = "";
  std::string decodedA = "";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, SingleChar) {
  std::string encodedA = "QQ==";
  std::string decodedA = "A";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, SingleCharNoEquals) {
  std::string encodedA = "QQ";
  std::string decodedA = "A";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, TwoChars) {
  std::string encodedA = "QUE=";
  std::string decodedA = "AA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, TwoCharsNoEquals) {
  std::string encodedA = "QUE";
  std::string decodedA = "AA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, ThreeChars) {
  std::string encodedA = "QUFB";
  std::string decodedA = "AAA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, FourChars) {
  std::string encodedA = "QUFBQQ";
  std::string decodedA = "AAAA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, FiveChars) {
  std::string encodedA = "QUFBQUE";
  std::string decodedA = "AAAAA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}

TEST(Base64urlDecoderTest, SixChars) {
  std::string encodedA = "QUFBQUFB";
  std::string decodedA = "AAAAAA";
  std::string output   = fromBase64url(encodedA);
  EXPECT_EQ(output, decodedA);
}
