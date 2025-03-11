#pragma once

#include "windowsLean.hpp"
#include <sql.h>
#include <string>

struct ParsedTimestamp {
    SQL_DATE_STRUCT date = {0};
    SQL_TIME_STRUCT time = {0};
    // SQL Timestamps include fractional seconds, but
    // the time struct does not. Therefore, we add it here.
    SQLUINTEGER fraction = 0;
};

ParsedTimestamp parseTimestamp(const std::string& input);

SQL_DATE_STRUCT parseDate(const std::string& input);

SQL_TIME_STRUCT parseTime(const std::string& input);
