#pragma once

#include "../../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, SQLSMALLINT>
    TRINO_RAW_TYPE_TO_ODBC_TYPE_CODE;

extern std::unordered_map<std::string, SQLLEN>
    TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES;

extern std::unordered_map<std::string, bool> TRINO_RAW_TYPE_TO_UNSIGNED;

extern std::unordered_map<std::string, SQLSMALLINT>
    TRINO_RAW_TYPE_TO_NUM_PREC_RADIX;

extern std::unordered_map<std::string, SQLCHAR> TRINO_RAW_TYPE_TO_PRECISION;
