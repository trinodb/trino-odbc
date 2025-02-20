#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/stringReplace.hpp"
#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

std::string descriptorFieldToParameterText(const DescriptorField& field) {
  switch (field.bufferCDataType) {
    // Assume SQL_C_CHAR is a varchar, and surround it with single quotes
    // to match the SQL standard. To avoid SQL injection, we need to escape
    // any single quotes in the string by replacing them with two single quotes.
    case SQL_C_CHAR: { // 1
      const std::string escaped =
          replaceAll(static_cast<char*>(field.bufferPtr), "'", "''");
      return std::format("'{}'", escaped);
    }
    // Assume all other types are numeric literals and do not require quotes.
    case SQL_C_FLOAT: { // 7
      return std::format("{}", *static_cast<float*>(field.bufferPtr));
    }
    case SQL_C_DOUBLE: { // 8
      return std::format("{}", *static_cast<double*>(field.bufferPtr));
    }
    case SQL_C_BIT: { // -7
      return std::format("{}", *static_cast<bool*>(field.bufferPtr));
    }
    case SQL_C_STINYINT:  // -26
    case SQL_C_TINYINT: { // -6
      return std::format("{}", *static_cast<int8_t*>(field.bufferPtr));
    }
    case SQL_C_UTINYINT: { // -28
      return std::format("{}", *static_cast<uint8_t*>(field.bufferPtr));
    }
    case SQL_C_SHORT:    // 5
    case SQL_C_SSHORT: { // -15
      return std::format("{}", *static_cast<int16_t*>(field.bufferPtr));
    }
    case SQL_C_USHORT: { // -17
      return std::format("{}", *static_cast<uint16_t*>(field.bufferPtr));
    }
    case SQL_C_LONG:    // 4
    case SQL_C_SLONG: { // -16
      return std::format("{}", *static_cast<int32_t*>(field.bufferPtr));
    }
    case SQL_C_ULONG: { // -18
      return std::format("{}", *static_cast<uint32_t*>(field.bufferPtr));
    }
    case SQL_C_SBIGINT: { // -25
      return std::format("{}", *static_cast<int64_t*>(field.bufferPtr));
    }
    case SQL_C_UBIGINT: { // -27
      return std::format("{}", *static_cast<uint64_t*>(field.bufferPtr));
    }
    case SQL_C_TIME: { // 10
      SQL_TIME_STRUCT* timeStruct =
          static_cast<SQL_TIME_STRUCT*>(field.bufferPtr);
      return std::format("TIME '{:02}:{:02}:{:02}'",
                         timeStruct->hour,
                         timeStruct->minute,
                         timeStruct->second);
    }
    case SQL_C_DATE: { // 9
      SQL_DATE_STRUCT* dateStruct =
          static_cast<SQL_DATE_STRUCT*>(field.bufferPtr);
      return std::format("DATE '{}-{:02}-{:02}'",
                         dateStruct->year,
                         dateStruct->month,
                         dateStruct->day);
    }
    case SQL_C_TIMESTAMP: { // 11
      SQL_TIMESTAMP_STRUCT* timestampStruct =
          static_cast<SQL_TIMESTAMP_STRUCT*>(field.bufferPtr);
      if (timestampStruct->fraction == 0) {
        return std::format("TIMESTAMP '{}-{:02}-{:02} {:02}:{:02}:{:02}'",
                           timestampStruct->year,
                           timestampStruct->month,
                           timestampStruct->day,
                           timestampStruct->hour,
                           timestampStruct->minute,
                           timestampStruct->second);
      } else {
        // While Trino supports up to 12 digits of sub-second precision, the
        // ODBC SQL_TIMESTAMP_STRUCT only supports 9 digits (nanosecond
        // precision). The 9 is what we have, so we'll pass the full precision
        // available.
        return std::format("TIMESTAMP '{}-{:02}-{:02} {:02}:{:02}:{:02}.{:09}'",
                           timestampStruct->year,
                           timestampStruct->month,
                           timestampStruct->day,
                           timestampStruct->hour,
                           timestampStruct->minute,
                           timestampStruct->second,
                           timestampStruct->fraction);
      }
    }
    default: {
      std::string errorMessage =
          std::format("Unsupported parameter type: {}", field.bufferCDataType);
      WriteLog(LL_ERROR, errorMessage);
      throw std::runtime_error(errorMessage);
    }
  }
}

SQLRETURN SQL_API SQLExecute(SQLHSTMT StatementHandle) {
  WriteLog(LL_DEBUG, "Entering SQLExecute");

  Statement* statementPtr = reinterpret_cast<Statement*>(StatementHandle);

  try {
    Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
    std::string lastPreparedStatementName =
        statement->trinoQuery->getLastPreparedStatementName();
    std::string query =
        std::format("EXECUTE \"{}\"\n", lastPreparedStatementName);

    // Pass each bound parameter if required, add a USING statement to
    // the first one, and a comma before all the others.
    Descriptor* paramDescriptor = statement->getParamDescriptor();
    if (paramDescriptor->getFieldCount() > 1) {
      query += "USING ";
      for (int i = 1; i < paramDescriptor->getFieldCount(); i++) {
        const DescriptorField& field = paramDescriptor->getFieldRef(i);
        if (i > 1) {
          query += ", ";
        }
        query += descriptorFieldToParameterText(field);
      }
    }
    WriteLog(LL_DEBUG, "  Executed Prepared Query: " + query);
    TrinoQuery* trinoQuery = statement->trinoQuery;
    WriteLog(LL_DEBUG, "  Setting Prepared Query");
    trinoQuery->setQuery(query);
    WriteLog(LL_DEBUG, "  POSTing Prepared Query");
    trinoQuery->post();
    WriteLog(LL_DEBUG, "  Setting prepared query to executed");
    statement->executed = true;
    return SQL_SUCCESS;
  } catch (const std::exception& ex) {
    WriteLog(LL_ERROR,
             "  ERROR: Exception thrown during SQLExecute: " +
                 std::string(ex.what()));
    // Set the diagnostic record on the statement handle
    // to make it possible to tell what happened here.
    ErrorInfo errorInfo("Exception thrown during SQLExecute: " +
                            std::string(ex.what()),
                        "HY000");
    statementPtr->setError(errorInfo);
    return SQL_ERROR;
  }
  // We should never get here, but let's have this in place just in case.
  return SQL_ERROR;
}
