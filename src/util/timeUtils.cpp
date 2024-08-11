#include "timeUtils.hpp"

#include <chrono>

long long getSecondsSinceEpoch() {
  const auto now                = std::chrono::system_clock::now();
  const auto durationSinceEpoch = now.time_since_epoch();
  const auto secondsSinceEpoch =
      std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch)
          .count();
  return secondsSinceEpoch;
}
