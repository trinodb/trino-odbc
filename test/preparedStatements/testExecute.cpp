#include <cstdint>
#include <gtest/gtest.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <windows.h>

#include "../../src/driver/execute.cpp"

// ---------------------------------------------------------------------------
// Helper: build a minimal DescriptorField for a given C type / buffer pointer.
// ---------------------------------------------------------------------------
static DescriptorField makeField(SQLSMALLINT cType, void* buf) {
  DescriptorField f;
  f.bufferCDataType = cType;
  f.bufferPtr       = buf;
  return f;
}

// ---------------------------------------------------------------------------
// SQL_C_CHAR – plain strings
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, CharSimpleString) {
  char value[]      = "hello";
  DescriptorField f = makeField(SQL_C_CHAR, value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "'hello'");
}

TEST(DescriptorFieldToParameterTextTest, CharEmptyString) {
  char value[]      = "";
  DescriptorField f = makeField(SQL_C_CHAR, value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "''");
}

// ---------------------------------------------------------------------------
// SQL_C_CHAR – SQL injection: embedded single quotes must be doubled
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, CharEscapesSingleQuote) {
  char value[]      = "O'Brien";
  DescriptorField f = makeField(SQL_C_CHAR, value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "'O''Brien'");
}

TEST(DescriptorFieldToParameterTextTest, CharEscapesMultipleSingleQuotes) {
  char value[]      = "it's a 'test'";
  DescriptorField f = makeField(SQL_C_CHAR, value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "'it''s a ''test'''");
}

TEST(DescriptorFieldToParameterTextTest, CharSqlInjectionAttempt) {
  // Classic injection: closing the string early then appending SQL
  char value[]      = "'; DROP TABLE users; --";
  DescriptorField f = makeField(SQL_C_CHAR, value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "'''; DROP TABLE users; --'");
}

// ---------------------------------------------------------------------------
// SQL_C_LONG – 32-bit signed integer
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, LongPositive) {
  int32_t value     = 42;
  DescriptorField f = makeField(SQL_C_LONG, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "42");
}

TEST(DescriptorFieldToParameterTextTest, LongNegative) {
  int32_t value     = -7;
  DescriptorField f = makeField(SQL_C_LONG, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "-7");
}

TEST(DescriptorFieldToParameterTextTest, LongZero) {
  int32_t value     = 0;
  DescriptorField f = makeField(SQL_C_LONG, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "0");
}

// ---------------------------------------------------------------------------
// SQL_C_FLOAT – single-precision floating point
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, FloatPositive) {
  float value       = 3.14f;
  DescriptorField f = makeField(SQL_C_FLOAT, &value);
  // std::format renders float with enough precision to round-trip; just
  // verify the returned string converts back to the same value.
  const std::string result = descriptorFieldToParameterText(f);
  EXPECT_FLOAT_EQ(std::stof(result), value);
}

TEST(DescriptorFieldToParameterTextTest, FloatNegative) {
  float value              = -1.5f;
  DescriptorField f        = makeField(SQL_C_FLOAT, &value);
  const std::string result = descriptorFieldToParameterText(f);
  EXPECT_FLOAT_EQ(std::stof(result), value);
}

TEST(DescriptorFieldToParameterTextTest, FloatZero) {
  float value       = 0.0f;
  DescriptorField f = makeField(SQL_C_FLOAT, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "0");
}

// ---------------------------------------------------------------------------
// SQL_C_TIME
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, Time) {
  SQL_TIME_STRUCT value = {1, 2, 3}; // 01:02:03
  DescriptorField f     = makeField(SQL_C_TIME, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "TIME '01:02:03'");
}

// ---------------------------------------------------------------------------
// SQL_C_DATE
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, Date) {
  SQL_DATE_STRUCT value = {2001, 8, 22}; // 2001-08-22
  DescriptorField f     = makeField(SQL_C_DATE, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f), "DATE '2001-08-22'");
}

// ---------------------------------------------------------------------------
// SQL_C_TIMESTAMP – without and with fractional seconds
// ---------------------------------------------------------------------------
TEST(DescriptorFieldToParameterTextTest, TimestampNoFraction) {
  SQL_TIMESTAMP_STRUCT value = {2020, 6, 10, 15, 55, 23, 0}; // no sub-second
  DescriptorField f          = makeField(SQL_C_TIMESTAMP, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f),
            "TIMESTAMP '2020-06-10 15:55:23'");
}

TEST(DescriptorFieldToParameterTextTest, TimestampWithFraction) {
  SQL_TIMESTAMP_STRUCT value = {
      2020, 6, 10, 15, 55, 23, 500000000}; // .5 seconds
  DescriptorField f = makeField(SQL_C_TIMESTAMP, &value);
  EXPECT_EQ(descriptorFieldToParameterText(f),
            "TIMESTAMP '2020-06-10 15:55:23.500000000'");
}
