#include "randomStr.hpp"

#include <random>
#include <vector>


const std::string CHARACTERS =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::string getRandomText(int length) {
  // Random device often provides hardware entropy based seeds (not PRNG).
  // However, it's often slow if called repeatedly in short succession.
  std::random_device rd;
  // Use the random_device to seed a PRNG that is fast.
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> dist(
      0, static_cast<int>(CHARACTERS.size() - 1));

  // Generate the random characters, leaving a null terminator
  // at the end.
  std::string randomText(length, '\0');
  for (int i = 0; i < length; i++) {
    int randomIndex = dist(generator);
    randomText[i]   = CHARACTERS[randomIndex];
  }

  return randomText;
}
