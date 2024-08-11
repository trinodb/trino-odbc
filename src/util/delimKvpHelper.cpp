#include "delimKvpHelper.hpp"
#include "stringTrim.hpp"
#include <algorithm>


std::map<std::string, std::string>
parseKVPsFromNullDelimChars(const char* kvpChars) {
  std::map<std::string, std::string> kvps;

  if (!kvpChars) {
    return kvps;
  }

  const char* keyStart = kvpChars;

  while (*keyStart != '\0') {
    const char* separator = keyStart;
    // Attributes are separated by an equals sign.
    while (*separator != '=' and *separator != '\0') {
      separator++;
    }

    if (*separator == '\0') {
      // This shouldn't happen. KVPs need an equals sign.
      break;
    }

    std::string key(keyStart, separator - keyStart);

    const char* valueStart = separator + 1;
    const char* valueEnd   = valueStart;

    // Find the next null character indicating the end
    // of the value
    while (*valueEnd != '\0') {
      valueEnd++;
    }

    std::string value(valueStart, valueEnd - valueStart);

    kvps[key] = value;
    keyStart  = valueEnd + 1;
  }

  return kvps;
}

std::string toLowercase(std::string s) {
  // This only works for ASCII text
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  return s;
}

std::string stripQuotes(std::string s, char quoteChar) {
  // Only strip quotes if they're on both ends of the string.
  if (!s.empty() and s.front() == quoteChar and s.back() == quoteChar) {
    s.erase(s.begin());
    s.pop_back();
  }
  return s;
}

std::map<std::string, std::string>
parseKVPsFromDelimStr(std::string kvpStr,
                      char delim,
                      bool lowercaseKeys,
                      bool trimKeysWhitespace,
                      bool stripDoubleQuotes) {
  std::map<std::string, std::string> kvps;

  if (kvpStr.empty()) {
    return kvps;
  }

  const char* keyStart = kvpStr.c_str();

  while (*keyStart != delim and *keyStart != '\0') {
    const char* separator = keyStart;
    // Attributes are separated by an equals sign.
    while (*separator != '=' and *separator != delim and *separator != '\0') {
      separator++;
    }
    if (*separator == delim or *separator == '\0') {
      // This shoudn't happen. KVPs require an equals sign.
      break;
    }

    std::string key(keyStart, separator - keyStart);

    const char* valueStart = separator + 1;
    const char* valueEnd   = valueStart;

    // Find the next null character indicating the end
    // of the value
    while (*valueEnd != delim and *valueEnd != '\0') {
      valueEnd++;
    }

    std::string value(valueStart, valueEnd - valueStart);

    // We might want to remove any whitespace from the keys
    if (trimKeysWhitespace) {
      trim(key);
    }

    if (stripDoubleQuotes) {
      key   = stripQuotes(key, '"');
      value = stripQuotes(value, '"');
    }

    if (lowercaseKeys) {
      key = toLowercase(key);
    }

    kvps[key] = value;
    keyStart  = valueEnd + 1;
  }

  return kvps;
}

std::map<std::string, std::string>
parseKVPsFromSemicolonDelimStr(std::string kvpStr) {
  char delimiter          = ';';
  bool lowercaseKeys      = true;
  bool trimKeysWhitespace = true;
  bool stripDoubleQuotes  = false;
  return parseKVPsFromDelimStr(
      kvpStr, delimiter, lowercaseKeys, trimKeysWhitespace, stripDoubleQuotes);
}

std::map<std::string, std::string>
parseKVPsFromCommaDelimStr(std::string kvpStr) {
  char delimiter          = ',';
  bool lowercaseKeys      = true;
  bool trimKeysWhitespace = true;
  bool stripDoubleQuotes  = true;
  return parseKVPsFromDelimStr(
      kvpStr, delimiter, lowercaseKeys, trimKeysWhitespace, stripDoubleQuotes);
}
