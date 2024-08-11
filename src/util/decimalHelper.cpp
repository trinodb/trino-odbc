#include "decimalHelper.hpp"

/*
I'm calling this lsbDecimalEncoder, but that's not really a good name for this.
It's implementing the ODBC numeric struct spec as defined in the MS KB article.

https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/retrieve-numeric-data-sql-numeric-struct-kb222831
*/
std::string lsbDecimalEncoder(std::string& s) {
  signed long long num = 0;
  // Negative numbers should skip parsing the '-' sign,
  // since the value
  if (s[0] == '-') {
    num = std::stoll(s.substr(1));
  } else {
    num = std::stoll(s);
  }

  std::string output;

  while (num > 0) {
    char byte = num & 0xFF;
    output.push_back(byte);
    num >>= 8;
  }

  return output;
}
