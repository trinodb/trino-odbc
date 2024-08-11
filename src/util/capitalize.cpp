#include "capitalize.hpp"

#include <algorithm>

/*
Return a capitalized version of the string str

If there are spaces in the string, capitalize the
character immediately following the spaces. Lowercase
all other characters.

Note: only works with single character encodings like ASCII.
*/
std::string capitalizedCase(std::string str) {
  // First, we lowercase the string using the built-in std::tolower.
  std::transform(str.begin(), str.end(), str.begin(), std::tolower);

  // Next, we capitalize the first letter of every word.
  // based on whether the prior character was a space.
  bool capitalizeNext = true;
  for (char& c : str) {
    if (capitalizeNext && std::isalpha(c)) {
      c              = std::toupper(c);
      capitalizeNext = false;
    } else if (std::isspace(c)) {
      capitalizeNext = true;
    }
  }

  return str;
}
