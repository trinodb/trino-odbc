#pragma once

#include <string>
#include <string_view>

// Replace all non-overlapping occurrences of `from` with `to` in `str`,
// returning the result as a new string.  If `from` is empty, `str` is
// returned unchanged.
std::string
replaceAll(std::string_view str, std::string_view from, std::string_view to);
