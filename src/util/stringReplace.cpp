#include "stringReplace.hpp"

std::string
replaceAll(std::string_view str, std::string_view from, std::string_view to) {
  if (from.empty()) {
    return std::string(str);
  }

  std::string result;
  result.reserve(str.size());

  std::string_view remaining = str;
  // Iterate through the string, finding each occurrence of `from`
  // and replacing it with `to`.
  while (true) {
    const auto pos = remaining.find(from);
    // If `from` is not found, append the rest of the string,
    // there are no more replacements to make.
    if (pos == std::string_view::npos) {
      result.append(remaining);
      break;
    }
    // Otherwise, append the part of the string before `from`,
    // then append `to`, and continue searching for the next
    // occurrence of `from` in the remaining string.
    result.append(remaining.substr(0, pos));
    result.append(to);
    remaining = remaining.substr(pos + from.size());
  }

  return result;
}
