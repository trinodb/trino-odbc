#include "dateAndTimeUtils.hpp"

#include <charconv>
#include <chrono>
#include <map>

std::map<std::string, const std::chrono::time_zone*> TIMEZONE_CACHE = {};

const std::chrono::time_zone* getTimezone(std::string& tzName) {
  /*
  Since we will be looking up a lot of timezones, it's probably a good idea
  to cache any responses in a variable.
  */
  if (TIMEZONE_CACHE.count(tzName) == 0) {
    const std::chrono::time_zone* zone =
        std::chrono::get_tzdb().locate_zone(tzName);
    TIMEZONE_CACHE[tzName] = zone;
  }
  return TIMEZONE_CACHE[tzName];
}

/*
How many characters into a standard trino timestamp do the
fractional seconds begin, if present?

We consider the beginning to be the decimal point, since
that's where parsing starts.

Date:   2000-01-01 00:00:00.123456789999
Index:  01234567890123456789
                           ^ - Position 19
*/
int FRACTIONAL_SECONDS_START_OFFSET = 19;

struct FractionParseResult {
    SQLUINTEGER fraction = 0;
    // We assume the fraction ends where it starts, which is true
    // for the base case (timestamps without a fractional second).
    int fractionEndIndex = FRACTIONAL_SECONDS_START_OFFSET;
};

FractionParseResult timestampFractionConverter(const char* timestampChars) {
  /*
  The fraction part of a timestamp can be anything from an empty
  string to 12 digits of text:

  ex1: 2000-01-01 00:00:00
  ex2: 2000-01-01 00:00:00.123456789999

  This function needs to always return the fraction component in the form
  of billionths of a second, no matter how many digits are provided. This
  may involve truncation for picosecond-precision timestamps, which are
  not supported by the ODBC spec.

  Ref:
  https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
  */
  FractionParseResult parseResult;
  const char* fractionStart = timestampChars + FRACTIONAL_SECONDS_START_OFFSET;

  // If there is no decimal point, we can assume the default
  // fraction value of zero. Otherwise we need to parse
  // the fraction value.
  if (*fractionStart != '.') {
    return parseResult;
  }

  // Find the end of the fraction
  const char* fractionEnd = fractionStart + 1;
  int fractionCharCount   = 1; // Include 1 for the decimal place.
  while (std::isdigit(*fractionEnd)) {
    fractionEnd++;
    fractionCharCount++;
  }

  // Now that we know the length of the fraction, save that. It's
  // required to parse the optional timezone component later.
  parseResult.fractionEndIndex =
      fractionCharCount + FRACTIONAL_SECONDS_START_OFFSET;

  // Once we know the start and end, we can pull the fraction
  // characters out into a c++ string and count them.
  std::string fractionStr(fractionStart + 1, fractionEnd);
  size_t fractionDigits     = fractionStr.length();
  const char* fractionChars = fractionStr.c_str();


  if (fractionDigits > 0 && fractionDigits <= 9) {
    // We need to parse the fraction as an integer
    // and perform the scaling operation to convert it to
    // billionths of a second.
    SQLUINTEGER fractionRawInt = 0;
    std::from_chars(
        fractionChars, fractionChars + fractionDigits, fractionRawInt);
    // Handle multiplying the number to get to billionths of a second.
    parseResult.fraction =
        fractionRawInt *
        static_cast<SQLUINTEGER>(std::pow(10, 9 - fractionDigits));
  } else if (fractionDigits > 9) {
    // Just truncate if there are more than 9 digits. Rounding would be
    // better, but that would require a 64-bit integer
    std::from_chars(fractionChars, fractionChars + 9, parseResult.fraction);
  }

  return parseResult;
}

std::string timezoneParser(const char* timestampChars,
                           size_t fractionEndIndex) {
  const char* timezoneStart = timestampChars + fractionEndIndex;

  // If there is no space character after the fraction, there is no
  // timezone on this timestamp. We can just return the empty string.
  if (*timezoneStart != ' ') {
    return "";
  } else {
    // Otherwise, consume the space character.
    timezoneStart += 1;
  }

  // Then, extract the end of the timezone string. This is easier said
  // than done. The following are valid IANA timezone db strings that
  // we should support. Some seem to be deprecated, but Trino supports them
  // so we should too. Notably, "Factory" is an IANA time zone that Trino
  // does not seem to support.
  //
  // * UTC
  // * Etc/GMT+1
  // * Etc/GMT+12
  // * Etc/GMT-1
  // * Etc/GMT-12
  // * Zulu
  // * PST8PDT
  //
  // This function needs to handle all these character classes gracefully. The
  // easiest way to do that is to rely on the terminating null character.
  const char* timezoneEnd = timezoneStart + 1;
  while (*timezoneEnd != '\0') {
    timezoneEnd++;
  }

  return std::string(timezoneStart, timezoneEnd);
}


ParsedTimestamp parseTimestamp(const std::string& input) {
  // Trino timestamps are strings with the following format
  //
  // YYYY-MM-DD HH:MM:SS[.FFF...] [TZ]
  //
  // Note that the fractional seconds and timezone are optional.
  // Fractional seconds will be present with the number of characters
  // specified in the timestamp, so a timestamp(0) will have no fractional
  // seconds, while a timestamp(12) will have 12 digits of fractional seconds.
  // Only "timestamp with time zone" types will have a timezone at the end.
  //
  //
  // ODBC times are a struct of several numbers
  //
  // SQLSMALLINT  year;
  // SQLUSMALLINT month;
  // SQLUSMALLINT day;
  // SQLUSMALLINT hour;
  // SQLUSMALLINT minute;
  // SQLUSMALLINT second;
  // SQLUINTEGER fraction;
  //
  // It makes sense to try to optimize a bit here because this
  // will get called many many times for a table containing many dates.
  //
  // Additionally, the timezone is an IANA Time Zone, not a UTC offset.
  // We want the driver to return timezone aware timestamps in UTC.
  // This allows the client to format these as they see fit. That means
  // we need to detect if there is a timezone and handle that appropriately.

  const char* valueChars = input.c_str();

  unsigned long yearRaw = 0;
  std::from_chars(valueChars + 0, valueChars + 4, yearRaw); // YYYY
  unsigned long monthRaw = 0;
  std::from_chars(valueChars + 5, valueChars + 7, monthRaw); // MM
  unsigned long dayRaw = 0;
  std::from_chars(valueChars + 8, valueChars + 10, dayRaw); // DD
  unsigned long hourRaw = 0;
  std::from_chars(valueChars + 11, valueChars + 13, hourRaw); // HH
  unsigned long minuteRaw = 0;
  std::from_chars(valueChars + 14, valueChars + 16, minuteRaw); // MM
  unsigned long secondRaw = 0;
  std::from_chars(valueChars + 17, valueChars + 19, secondRaw); // SS

  // [.FFF...]
  FractionParseResult fractionParse = timestampFractionConverter(valueChars);

  // [TZ]
  std::string timezoneName =
      timezoneParser(valueChars, fractionParse.fractionEndIndex);

  // Now that we have all the raw info, we can construct timezone
  // aware time and date objects.
  std::chrono::year_month_day ymd = {std::chrono::year(yearRaw),
                                     std::chrono::month(monthRaw),
                                     std::chrono::day(dayRaw)};
  std::chrono::local_days localDaysSinceEpoch(ymd);

  auto totalNanoseconds = std::chrono::nanoseconds(
      hourRaw * 3'600'000'000'000LL + minuteRaw * 60'000'000'000LL +
      secondRaw * 1'000'000'000LL + fractionParse.fraction);

  std::chrono::local_time<std::chrono::nanoseconds> localTimestamp =
      std::chrono::local_time<std::chrono::nanoseconds>(localDaysSinceEpoch) +
      totalNanoseconds;

  // Maybe apply a timezone offset.
  if (timezoneName.empty()) {
    // If there's no timezone, assume the timestamp is already in UTC.
    timezoneName = "Etc/UTC";
  }
  // Create a timezone aware time object.
  const std::chrono::time_zone* tz = getTimezone(timezoneName);
  std::chrono::zoned_time<std::chrono::nanoseconds> zonedTime(tz,
                                                              localTimestamp);
  // Overwrite the timestamp with a version in "sys_time", which
  // is documented to be unspecified, but most implementations use Unix Time,
  // which is UTC-zoned.
  std::chrono::sys_time<std::chrono::nanoseconds> utcTime =
      zonedTime.get_sys_time();

  // Convert our timestamp to a date/time suitable to return.
  std::chrono::year_month_day ymdOut =
      std::chrono::floor<std::chrono::days>(utcTime);
  std::chrono::hh_mm_ss hmsOut(utcTime - std::chrono::sys_days(ymdOut));

  // Last, we put all the times and dates into our
  // ParsedTimestamp result object to return to the rest of the codebase.
  ParsedTimestamp result = {0};
  result.date.year       = static_cast<int>(ymdOut.year());
  result.date.month      = static_cast<unsigned>(ymdOut.month());
  result.date.day        = static_cast<unsigned>(ymdOut.day());
  result.time.hour       = static_cast<unsigned>(hmsOut.hours().count());
  result.time.minute     = static_cast<unsigned>(hmsOut.minutes().count());
  result.time.second     = static_cast<unsigned>(hmsOut.seconds().count());
  result.fraction        = static_cast<unsigned>(hmsOut.subseconds().count());

  return result;
}


SQL_DATE_STRUCT parseDate(const std::string& input) {
  // Trino dates are strings, "YYYY-MM-DD".
  //
  // ODBC dates are a struct of three numbers
  // SQLSMALLINT year;
  // SQLUSMALLINT month;
  // SQLUSMALLINT day;
  //
  // It makes sense to try to optimize a bit here because this
  // will get called many many times for a table containing many dates.
  const char* valueChars = input.c_str();

  SQL_DATE_STRUCT result = {0};

  std::from_chars(valueChars + 0, valueChars + 4, result.year);  // YYYY
  std::from_chars(valueChars + 5, valueChars + 7, result.month); // MM
  std::from_chars(valueChars + 8, valueChars + 10, result.day);  // DD

  return result;
}


SQL_TIME_STRUCT parseTime(const std::string& input) {
  // Trino times are strings, "HH:MM:SS.FFF" where
  // FFF is fractions of a second.
  //
  // ODBC times are a struct of three numbers
  // SQLUSMALLINT hour;
  // SQLUSMALLINT minute;
  // SQLUSMALLINT second;
  //
  // It makes sense to try to optimize a bit here because this
  // will get called many many times for a table containing many dates.
  // The basic SQL_C_TIME type does not support fractions of a second,
  // so that information ends up being discarded.

  const char* valueChars = input.c_str();

  SQL_TIME_STRUCT result = {0};

  std::from_chars(valueChars + 0, valueChars + 2, result.hour);   // HH
  std::from_chars(valueChars + 3, valueChars + 5, result.minute); // MM
  std::from_chars(valueChars + 6, valueChars + 8, result.second); // SS

  return result;
}
