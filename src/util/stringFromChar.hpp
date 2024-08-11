#pragma once
#include <string>

// Stealing the type code from sql.h for null terminated strings,
// but without taking a dependency on the sql.h header.
constexpr long CHAR_IS_NTS = -3;

std::string stringFromChar(char* text, long textLength);
std::string stringFromChar(unsigned char* text, long textLength);
