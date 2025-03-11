#include <gtest/gtest.h>
#include <string>

#include "../../../src/util/dateAndTimeUtils.hpp"

TEST(DateAndTimeUtilsTest, Date) {
  std::string str        = "2025-03-10";
  SQL_DATE_STRUCT parsed = parseDate(str);
  EXPECT_EQ(parsed.year, 2025);
  EXPECT_EQ(parsed.month, 3);
  EXPECT_EQ(parsed.day, 10);
}

TEST(DateAndTimeUtilsTest, Time) {
  // SQL TIME does not include fractional seconds, even
  // though Trino returns fractional seconds. We have to
  // just drop that part.
  std::string str        = "12:34:56.789";
  SQL_TIME_STRUCT parsed = parseTime(str);
  EXPECT_EQ(parsed.hour, 12);
  EXPECT_EQ(parsed.minute, 34);
  EXPECT_EQ(parsed.second, 56);
}

TEST(DateAndTimeUtilsTest, Timestamp) {
  std::string str        = "2025-03-10 12:34:56.789";
  ParsedTimestamp parsed = parseTimestamp(str);
  EXPECT_EQ(parsed.date.year, 2025);
  EXPECT_EQ(parsed.date.month, 3);
  EXPECT_EQ(parsed.date.day, 10);
  EXPECT_EQ(parsed.time.hour, 12);
  EXPECT_EQ(parsed.time.minute, 34);
  EXPECT_EQ(parsed.time.second, 56);
  EXPECT_EQ(parsed.fraction, 789000000);
}

TEST(DateAndTimeUtilsTest, TimestampWithPicoseconds) {
  // Trino supports picosecond precision (12 digits), but ODBC
  // only supports nanosecond precision (9 digits). The behavior
  // we want to implement for this is simple digit truncation.
  std::string str        = "2025-03-10 12:34:56.111222333444";
  ParsedTimestamp parsed = parseTimestamp(str);
  EXPECT_EQ(parsed.date.year, 2025);
  EXPECT_EQ(parsed.date.month, 3);
  EXPECT_EQ(parsed.date.day, 10);
  EXPECT_EQ(parsed.time.hour, 12);
  EXPECT_EQ(parsed.time.minute, 34);
  EXPECT_EQ(parsed.time.second, 56);
  EXPECT_EQ(parsed.fraction, 111222333);
}

TEST(DateAndTimeUtilsTest, TimestampNoSeconds) {
  std::string str        = "2025-03-10 12:34:56";
  ParsedTimestamp parsed = parseTimestamp(str);
  EXPECT_EQ(parsed.date.year, 2025);
  EXPECT_EQ(parsed.date.month, 3);
  EXPECT_EQ(parsed.date.day, 10);
  EXPECT_EQ(parsed.time.hour, 12);
  EXPECT_EQ(parsed.time.minute, 34);
  EXPECT_EQ(parsed.time.second, 56);
  EXPECT_EQ(parsed.fraction, 0);
}


TEST(DateAndTimeUtilsTest, TimestampWithNonUTCTimezone) {
  // This timezone is 6 hours before UTC, so it actually changes
  // the date to the next day.
  std::string str        = "2025-03-09 23:01:02.3 America/Chicago";
  ParsedTimestamp parsed = parseTimestamp(str);
  EXPECT_EQ(parsed.date.year, 2025);
  EXPECT_EQ(parsed.date.month, 3);
  EXPECT_EQ(parsed.date.day, 10);
  EXPECT_EQ(parsed.time.hour, 4);
  EXPECT_EQ(parsed.time.minute, 1);
  EXPECT_EQ(parsed.time.second, 2);
  EXPECT_EQ(parsed.fraction, 300000000);
}


TEST(DateAndTimeUtilsTest, TimestampWithUTCTimezone) {
  std::string str        = "2025-03-10 12:34:56.789 UTC";
  ParsedTimestamp parsed = parseTimestamp(str);
  EXPECT_EQ(parsed.date.year, 2025);
  EXPECT_EQ(parsed.date.month, 3);
  EXPECT_EQ(parsed.date.day, 10);
  EXPECT_EQ(parsed.time.hour, 12);
  EXPECT_EQ(parsed.time.minute, 34);
  EXPECT_EQ(parsed.time.second, 56);
  EXPECT_EQ(parsed.fraction, 789000000);
}
