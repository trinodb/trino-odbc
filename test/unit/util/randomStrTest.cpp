#include "gtest/gtest.h"
#include <string>

#include "../../../src/util/randomStr.hpp"

TEST(RandomStrTest, IsCorrectLength) {
  std::string randomness = getRandomText(5);

  ASSERT_NE(randomness, "");
  ASSERT_EQ(randomness.size(), 5);
}
