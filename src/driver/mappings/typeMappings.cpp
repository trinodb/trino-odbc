#include "typeMappings.hpp"

std::unordered_map<std::string, SQLSMALLINT> TRINO_RAW_TYPE_TO_ODBC_TYPE_CODE =
    {
        std::make_pair("bigint", SQL_BIGINT),
        std::make_pair("integer", SQL_INTEGER),
        std::make_pair("smallint", SQL_SMALLINT),
        std::make_pair("tinyint", SQL_TINYINT),
        std::make_pair("double", SQL_DOUBLE),
        std::make_pair("real", SQL_REAL),
        std::make_pair("boolean", SQL_BIT),
        std::make_pair("varchar", SQL_VARCHAR),
        std::make_pair("uuid", SQL_GUID),
        std::make_pair("decimal", SQL_DECIMAL),
        std::make_pair("date", SQL_TYPE_DATE),
        std::make_pair("time", SQL_TYPE_TIME),
        std::make_pair("timestamp", SQL_TYPE_TIMESTAMP),
        std::make_pair("timestamp with time zone", SQL_TYPE_TIMESTAMP),
};

std::unordered_map<std::string, SQLLEN> TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES = {
    std::make_pair("bigint", 8),
    std::make_pair("integer", 4),
    std::make_pair("smallint", 2),
    std::make_pair("tinyint", 1),
    std::make_pair("double", 8),
    std::make_pair("real", 4),
    std::make_pair("boolean", 1),
    std::make_pair("varchar", SQL_NO_TOTAL),
    std::make_pair("uuid", sizeof(SQLGUID)),
    std::make_pair("decimal", SQL_NO_TOTAL),
    std::make_pair("date", sizeof(SQL_DATE_STRUCT)),
    std::make_pair("time", sizeof(SQL_TIME_STRUCT)),
    std::make_pair("timestamp", sizeof(SQL_TIMESTAMP_STRUCT)),
    std::make_pair("timestamp with time zone", sizeof(SQL_TIMESTAMP_STRUCT)),
};

/*
Trino integers are all signed (true). If the integers are unsigned in the
source, trino will convert them to the next largest size of signed integer, or
a decimal if it wouldn't fit into an int64.

All other types are unsigned (false).
*/
std::unordered_map<std::string, bool> TRINO_RAW_TYPE_TO_UNSIGNED = {
    std::make_pair("bigint", false),
    std::make_pair("integer", false),
    std::make_pair("smallint", false),
    std::make_pair("tinyint", false),
    std::make_pair("double", false),
    std::make_pair("real", false),
    std::make_pair("boolean", true),
    std::make_pair("varchar", true),
    std::make_pair("uuid", true),
    std::make_pair("decimal", false),
    std::make_pair("date", true),
    std::make_pair("time", true),
    std::make_pair("timestamp", true),
    std::make_pair("timestamp with time zone", true),
};

/*
  Decimal is the numeric type that supports base-10 number systems.
  All other types specify precision in base-2.
*/
std::unordered_map<std::string, SQLSMALLINT> TRINO_RAW_TYPE_TO_NUM_PREC_RADIX =
    {
        std::make_pair("decimal", 10),
        std::make_pair("bigint", 2),
        std::make_pair("integer", 2),
        std::make_pair("smallint", 2),
        std::make_pair("tinyint", 2),
        std::make_pair("double", 2),
        std::make_pair("float", 2),
};

/*
  Precision for these types is all in base-2 units (bits of precision).
  Decimal precision is base-10, but varies per column. We can't use a
  lookup table for decimals.
*/
std::unordered_map<std::string, SQLCHAR> TRINO_RAW_TYPE_TO_PRECISION = {
    std::make_pair("bigint", 64),
    std::make_pair("integer", 32),
    std::make_pair("smallint", 16),
    std::make_pair("tinyint", 8),
    std::make_pair("double", 53),
    std::make_pair("float", 24),
};
