#include "rowToBuffer.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "decimalHelper.hpp"
#include "writeLog.hpp"

ColumnToBufferStatus::ColumnToBufferStatus(bool isSuccess,
                                           bool isVariableLength) {
  this->isSuccess        = isSuccess;
  this->isVariableLength = isVariableLength;
}

SQLRETURN copyStrToBuffer(const json& rowData,
                          SQLULEN columnNumber,
                          void* buffer,
                          SQLLEN bufferLength,
                          SQLLEN* strLen_or_IndPtr,
                          std::string cTypeName) {
  try {
    std::string value = rowData[columnNumber - 1].get<std::string>();
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE, "  Detected bound " + cTypeName + " : " + value);
    }

    // We need to be sure not to copy past the end of the buffer.
    SQLLEN copyLength = 0;
    if (bufferLength > 0) {
      copyLength = std::min<SQLLEN>(value.size(), bufferLength - 1);
    }

    // Copy characters into the buffer up to the calculated end.
    std::memcpy(buffer, value.c_str(), copyLength);

    // Don't forget a null terminating char at the end.
    if (bufferLength > 0) {
      static_cast<char*>(buffer)[copyLength] = '\0';
    }

    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = static_cast<SQLLEN>(value.size());
    }
    return SQL_SUCCESS;
  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting " + cTypeName + " value for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}


template <typename T>
SQLRETURN copyFixedLenToBuffer(const json& rowData,
                               SQLULEN columnNumber,
                               void* buffer,
                               SQLLEN* strLen_or_IndPtr,
                               std::string cTypeName) {
  try {
    const json& jsonValue = rowData[columnNumber - 1];
    T value               = jsonValue.get<T>();
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE,
               "  Detected bound " + cTypeName + " : " + std::to_string(value));
    }
    *reinterpret_cast<T*>(buffer) = value;
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = sizeof(T);
    }
    return SQL_SUCCESS;

  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting " + cTypeName + " value for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}


SQLRETURN copyDateToBuffer(SQLULEN columnNumber,
                           SQLSMALLINT year,
                           SQLUSMALLINT month,
                           SQLUSMALLINT day,
                           void* buffer,
                           SQLLEN bufferLength,
                           SQLLEN* strLen_or_IndPtr) {
  try {
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE,
               "  Detected bound date: " + std::to_string(year) + '-' +
                   std::to_string(month) + '-' + std::to_string(day));
    }
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = 6; // Six bytes in a date struct.
    }
    SQL_DATE_STRUCT* datePtr = reinterpret_cast<SQL_DATE_STRUCT*>(buffer);
    datePtr->year            = year;
    datePtr->month           = month;
    datePtr->day             = day;
    return SQL_SUCCESS;

  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting date value for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}


SQLRETURN copyTimeToBuffer(SQLULEN columnNumber,
                           SQLUSMALLINT hour,
                           SQLUSMALLINT minute,
                           SQLUSMALLINT second,
                           void* buffer,
                           SQLLEN bufferLength,
                           SQLLEN* strLen_or_IndPtr) {
  try {
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE,
               "  Detected bound time: " + std::to_string(hour) + '-' +
                   std::to_string(minute) + '-' + std::to_string(second));
    }
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = 6; // Six bytes in a time struct.
    }
    SQL_TIME_STRUCT* timePtr = reinterpret_cast<SQL_TIME_STRUCT*>(buffer);
    timePtr->hour            = hour;
    timePtr->minute          = minute;
    timePtr->second          = second;
    return SQL_SUCCESS;

  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting time value for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}

SQLUINTEGER timestampFractionConverter(const char* timestampChars) {
  /*
  The fraction part of a timestamp can be anything from an empty
  string to 12 digits of text:

  ex1: 2000-01-01 00:00:00
  ex2: 2000-01-01 00:00:00.123456789999

  This function needs to always return the fraction component in the form
  of billionths of a second, no matter how many digits are provided.

  Ref:
  https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
  */
  SQLUINTEGER fraction      = 0;
  const char* fractionStart = timestampChars + 19;

  // If there is no decimal point, we can assume the default
  // fraction value of zero. Otherwise we need to parse
  // the fraction value.
  if (*fractionStart != '.') {
    return fraction;
  }

  // Find the end of the fraction
  const char* fractionEnd = fractionStart + 1;
  while (std::isdigit(*fractionEnd)) {
    fractionEnd++;
  }

  // Once we know the start and end, we can pull the fraction
  // characters out into a c++ string and count them.
  std::string fractionStr(fractionStart + 1, fractionEnd);
  size_t fractionDigits = fractionStr.length();

  // Last we need to parse the fraction as an integer
  // and perform the scaling operation to convert it to
  // billionths of a second.
  SQLUINTEGER fractionRawInt = std::strtoul(fractionStr.c_str(), nullptr, 10);

  if (fractionDigits > 0 && fractionDigits <= 9) {
    // Handle multiplying the number to get to billionths of a second.
    fraction = fractionRawInt *
               static_cast<SQLUINTEGER>(std::pow(10, 9 - fractionDigits));
  } else if (fractionDigits > 9) {
    // Just truncate if there are more than 9 digits. Rounding would be
    // better, but that would require a 64-bit integer
    fraction = std::strtoul(fractionStr.substr(0, 9).c_str(), nullptr, 10);
  }

  return fraction;
}

SQLRETURN copyTimestampToBuffer(SQLULEN columnNumber,
                                SQLSMALLINT year,
                                SQLUSMALLINT month,
                                SQLUSMALLINT day,
                                SQLUSMALLINT hour,
                                SQLUSMALLINT minute,
                                SQLUSMALLINT second,
                                SQLUINTEGER fraction,
                                void* buffer,
                                SQLLEN bufferLength,
                                SQLLEN* strLen_or_IndPtr) {
  try {
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE,
               "  Detected bound time: " + std::to_string(hour) + '-' +
                   std::to_string(minute) + '-' + std::to_string(second));
    }
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = 6; // Six bytes in a time struct.
    }
    SQL_TIMESTAMP_STRUCT* timestampPtr =
        reinterpret_cast<SQL_TIMESTAMP_STRUCT*>(buffer);
    timestampPtr->year     = year;
    timestampPtr->month    = month;
    timestampPtr->day      = day;
    timestampPtr->hour     = hour;
    timestampPtr->minute   = minute;
    timestampPtr->second   = second;
    timestampPtr->fraction = fraction;
    return SQL_SUCCESS;

  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting time value for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}

SQLRETURN copyDecimalToBuffer(SQLULEN columnNumber,
                              const char* valueChars,
                              void* buffer,
                              SQLLEN bufferLength,
                              SQLLEN* strLen_or_IndPtr,
                              SQLCHAR precision,
                              SQLCHAR scale) {
  try {
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = sizeof(tagSQL_NUMERIC_STRUCT);
    }

    std::string valueString = valueChars;

    std::string wholePartStr      = "";
    std::string fractionalPartStr = "";
    bool isPositive               = true;

    // First, determine if the number is negative by looking
    // at the first character of the string.
    if (valueString[0] == '-') {
      isPositive = false;
    }

    // Use the position of the decimal place if it exists
    // to parse the whole and fractional component of the decimal.
    size_t dotPosition = valueString.find('.');
    if (dotPosition == std::string::npos) {
      wholePartStr = valueString;
    } else {
      wholePartStr      = valueString.substr(0, dotPosition);
      fractionalPartStr = valueString.substr(dotPosition + 1);
    }

    // Interpret the buffer as a decimal struct.
    tagSQL_NUMERIC_STRUCT* numeric =
        reinterpret_cast<tagSQL_NUMERIC_STRUCT*>(buffer);

    // We might as well use the passed-in precision and scale to
    // set those values. The precision in particular cannot be
    // inferred from the string representation of the number.
    // "1" could have precision 1 or precision 38. The string
    // representation is identical in both cases. Scale
    // usually has trailing zeros that could be used to infer
    // the scale, but why not just read it from the column
    // metadata like the precision?
    numeric->precision = precision;
    numeric->scale     = scale;
    numeric->sign      = isPositive;

    // Last, set the val array on the struct in hex format.
    std::string stringValue     = wholePartStr + fractionalPartStr;
    std::string lsbEncodedValue = lsbDecimalEncoder(stringValue);
    size_t valueLength          = std::max(lsbEncodedValue.size(),
                                  static_cast<size_t>(SQL_MAX_NUMERIC_LEN));
    std::fill_n(numeric->val, SQL_MAX_NUMERIC_LEN, '\0');
    std::memcpy(numeric->val, lsbEncodedValue.c_str(), valueLength);

    return SQL_SUCCESS;
  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting decimal for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}

SQLRETURN copyGuidToBuffer(SQLULEN columnNumber,
                           const char* valueChars,
                           void* buffer,
                           SQLLEN bufferLength,
                           SQLLEN* strLen_or_IndPtr) {
  try {
    if (strLen_or_IndPtr) {
      // Sixteen bytes in a GUID.
      *strLen_or_IndPtr = sizeof(SQLGUID);
    }
    std::stringstream ss(valueChars);
    unsigned int data1        = 0;
    unsigned short data2      = 0;
    unsigned short data3      = 0;
    unsigned short data4Part1 = 0;
    int64_t data4Part2        = 0;

    char dash;

    // Parse the dword part.
    ss >> std::hex >> data1 >> dash;
    // Parse the first word part.
    ss >> std::hex >> data2 >> dash;
    // Parse the second word part.
    ss >> std::hex >> data3 >> dash;
    // Parse the first 4 characters of the last data component.
    ss >> std::hex >> data4Part1 >> dash;
    // Parse the last 12 bytes (6 hex characters) into the byte array
    ss >> std::hex >> data4Part2;

    // Combine the bits from the two different data4 parts
    // into a combined byte array. To preserve correct endianness, we
    // have to handle this one byte at a time.
    unsigned char data4Combined[8];
    // The first 4 chars (2 bytes)
    data4Combined[0] = (data4Part1 >> 8) & 0xFF;
    data4Combined[1] = (data4Part1 >> 0) & 0xFF;
    // The last 12 chars (6 bytes)
    data4Combined[2] = (data4Part2 >> 40) & 0xFF;
    data4Combined[3] = (data4Part2 >> 32) & 0xFF;
    data4Combined[4] = (data4Part2 >> 24) & 0xFF;
    data4Combined[5] = (data4Part2 >> 16) & 0xFF;
    data4Combined[6] = (data4Part2 >> 8) & 0xFF;
    data4Combined[7] = (data4Part2 >> 0) & 0xFF;

    SQLGUID* guid = reinterpret_cast<SQLGUID*>(buffer);
    guid->Data1   = data1;
    guid->Data2   = data2;
    guid->Data3   = data3;
    std::memcpy(guid->Data4, data4Combined, 8);

    return SQL_SUCCESS;
  } catch (const std::exception& e) {
    WriteLog(LL_ERROR,
             "  ERROR: extracting GUID for column index: " +
                 std::to_string(columnNumber) + " - " + e.what());
    return SQL_ERROR;
  }
}

ColumnToBufferStatus columnToBuffer(SQLSMALLINT cDataType,
                                    SQLSMALLINT odbcDataType,
                                    const json& rowData,
                                    SQLULEN columnNumber,
                                    void* buffer,
                                    SQLLEN bufferLength,
                                    SQLLEN* strLen_or_IndPtr,
                                    SQLCHAR precision,
                                    SQLCHAR scale) {
  switch (cDataType) {
    case SQL_C_CHAR: { // 1
      // Char pointers are used in a bunch of different ways. How to use
      // it might depend on the ODBC type. If we need to do something
      // dynamically based on the SQL data type, this is where it would happen.
      // For now, we're treating everything as a varchar. This seems to work
      // for GUIDs and Decimals as well.
      copyStrToBuffer(rowData,
                      columnNumber,
                      buffer,
                      bufferLength,
                      strLen_or_IndPtr,
                      "CHAR");
      return ColumnToBufferStatus(true, true);
    }
    case SQL_C_NUMERIC: { // 2
      // Trino decimals return as strings, '123.456'
      std::string value      = rowData[columnNumber - 1].get<std::string>();
      const char* valueChars = value.c_str();

      copyDecimalToBuffer(columnNumber,
                          valueChars,
                          buffer,
                          bufferLength,
                          strLen_or_IndPtr,
                          precision,
                          scale);

      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_GUID: { // -11
      // Trino guids are strings, "00000000-0000-0000-0000-000000000000"
      std::string value      = rowData[columnNumber - 1].get<std::string>();
      const char* valueChars = value.c_str();

      copyGuidToBuffer(
          columnNumber, valueChars, buffer, bufferLength, strLen_or_IndPtr);

      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_DATE:        // 9
    case SQL_C_TYPE_DATE: { // 91
      // Trino dates are strings, "YYYY-MM-DD".
      //
      // ODBC dates are a struct of three numbers
      // SQLSMALLINT year;
      // SQLUSMALLINT month;
      // SQLUSMALLINT day;
      //
      // It makes sense to try to optimize a bit here because this
      // will get called many many times for a table containing many dates.

      std::string value      = rowData[columnNumber - 1].get<std::string>();
      const char* valueChars = value.c_str();

      SQLSMALLINT year = static_cast<SQLSMALLINT>(
          std::strtol(valueChars + 0, nullptr, 10)); // YYYY
      SQLUSMALLINT month = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 5, nullptr, 10)); // MM
      SQLUSMALLINT day = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 8, nullptr, 10)); // DD
      copyDateToBuffer(columnNumber,
                       year,
                       month,
                       day,
                       buffer,
                       bufferLength,
                       strLen_or_IndPtr);
      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_TIME:        // 10
    case SQL_C_TYPE_TIME: { // 92
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

      std::string value      = rowData[columnNumber - 1].get<std::string>();
      const char* valueChars = value.c_str();

      SQLUSMALLINT hour = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 0, nullptr, 10)); // HH
      SQLUSMALLINT minute = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 3, nullptr, 10)); // MM
      SQLUSMALLINT second = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 6, nullptr, 10)); // SS

      copyTimeToBuffer(columnNumber,
                       hour,
                       minute,
                       second,
                       buffer,
                       bufferLength,
                       strLen_or_IndPtr);

      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_TIMESTAMP:        // 11
    case SQL_C_TYPE_TIMESTAMP: { // 93
      // Trino timestamps are strings, "YYYY-MM-DD HH:MM:SS.FFF[...]"
      //
      // ODBC times are a struct of several numbers

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
      // The basic SQL_C_TIME type does not support fractions of a second,
      // so that information ends up being discarded.

      std::string value      = rowData[columnNumber - 1].get<std::string>();
      const char* valueChars = value.c_str();

      SQLSMALLINT year = static_cast<SQLSMALLINT>(
          std::strtol(valueChars + 0, nullptr, 10)); // YYYY
      SQLUSMALLINT month = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 5, nullptr, 10)); // MM
      SQLUSMALLINT day = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 8, nullptr, 10)); // DD
      SQLUSMALLINT hour = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 11, nullptr, 10)); // HH
      SQLUSMALLINT minute = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 14, nullptr, 10)); // MM
      SQLUSMALLINT second = static_cast<SQLUSMALLINT>(
          std::strtol(valueChars + 17, nullptr, 10));                // SS
      SQLUINTEGER fraction = timestampFractionConverter(valueChars); // FFF[...]

      copyTimestampToBuffer(columnNumber,
                            year,
                            month,
                            day,
                            hour,
                            minute,
                            second,
                            fraction,
                            buffer,
                            bufferLength,
                            strLen_or_IndPtr);

      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_BIT:        // -7
    case SQL_C_TINYINT:    // -6
    case SQL_C_STINYINT: { // -26
      copyFixedLenToBuffer<int8_t>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "BIT/[S]TINYINT");
      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_SHORT:    // 5
    case SQL_C_SSHORT: { // -15
      copyFixedLenToBuffer<int16_t>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "SHORT/SSHORT");
      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_LONG:    // 4
    case SQL_C_SLONG: { // -16
      copyFixedLenToBuffer<int32_t>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "LONG/SLONG");
      return ColumnToBufferStatus(true, false);
    }
    case SQL_BIGINT:      // -5
    case SQL_C_SBIGINT: { // -25
      copyFixedLenToBuffer<int64_t>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "BIGINT/SBIGINT");
      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_FLOAT: { // 7
      copyFixedLenToBuffer<float>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "FLOAT");
      return ColumnToBufferStatus(true, false);
    }
    case SQL_C_DOUBLE: { // 8
      copyFixedLenToBuffer<double>(
          rowData, columnNumber, buffer, strLen_or_IndPtr, "DOUBLE");
      return ColumnToBufferStatus(true, false);
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: Cannot handle bound column for column index: " +
                   std::to_string(columnNumber));
      WriteLog(LL_ERROR,
               "  ERROR: Column type detected as: " +
                   std::to_string(cDataType));
      return ColumnToBufferStatus(false, false);
    }
  }
}
