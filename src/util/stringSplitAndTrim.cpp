#include "stringSplitAndTrim.hpp"
#include "stringTrim.hpp"
#include <algorithm>
#include <sstream>


std::vector<std::string> stringSplitAndTrim(const std::string& input,
                                            const char delimiter) {
  std::vector<std::string> result;
  std::istringstream ss(input);
  std::string token;

  while (ss.good()) {
    std::getline(ss, token, delimiter);
    // Remove any space/tab characters from beginning and end.
    trim(token);
    if (!token.empty()) {
      result.push_back(token);
    }
  }

  return result;
}
