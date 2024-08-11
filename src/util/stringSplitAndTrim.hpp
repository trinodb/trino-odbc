#pragma once

#include <string>
#include <vector>

// Given a comma-delimited string like "ABC, DEF", return the individual tokens
// within that string, with whitespace removed.
std::vector<std::string> stringSplitAndTrim(const std::string& input,
                                            const char delimiter);
