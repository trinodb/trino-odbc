#include "stringTrim.hpp"

#include <algorithm>
#include <cctype>

/*
 Analogous to to python's str.trim(), but in-place.
*/
void trim(std::string& str) {
  // Trim whitespace from the start of the string
  auto firstRealChar = std::ranges::find_if_not(str, std::isspace);
  str.erase(str.begin(), firstRealChar);

  // Use reverse-iterator to find the last non-space character
  auto lastRealChar =
      std::ranges::find_if_not(str.rbegin(), str.rend(), std::isspace).base();
  str.erase(lastRealChar, str.end());
}
