#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/stringFromChar.hpp"
#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

std::string constructColumnQuery(std::string catalog,
                                 std::string schema,
                                 std::string tableName,
                                 std::string columnName) {
  /*
  We need to implement a result set that matches this exact spec.
  https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlcolumns-function

  There are a few key differences from the JDBC defaults:
  * Trino type 2014 is timestamp with time zone in system.jdbc.columns. This is
    non-standard.
      * We substitute data_type 93 - SQL_TYPE_TIMESTAMP
      * We substitute type_name 'timestamp'
      * We substitute column_size 16 - sizeof(SQL_TIMESTAMP_STRUCT)
  * ODBC expects decimal_digits to be 0 for integral numeric types, never NULL.
      * Reference:
  https://learn.microsoft.com/en-us/sql/odbc/reference/appendixes/decimal-digits
      * Problem: system.jdbc.columns reports integral types with NULL decimal
        digits
      * Solution: explicitly set integral types to have 0 decimal digits
  */
  // clang-format off
  std::string query = std::string(R"SQL(
    SELECT
        table_cat,
        table_schem,
        table_name,
        column_name,
        CASE data_type
            WHEN 2014 THEN 93
            ELSE data_type
        END AS data_type,
        CASE data_type
            WHEN 2014 THEN 'timestamp'
            ELSE type_name
        END AS type_name,
        CASE data_type
            WHEN 2014 THEN 16
            ELSE column_size
        END AS column_size,
        buffer_length,
        CASE
            WHEN type_name IN ('bigint', 'integer', 'smallint', 'tinyint') THEN 0
            ELSE decimal_digits
        END AS decimal_digits,
        num_prec_radix,
        nullable,
        remarks,
        column_def,
        sql_data_type,
        sql_datetime_sub,
        char_octet_length,
        ordinal_position,
        is_nullable
   FROM
        system.jdbc.columns
   WHERE 1=1
  )SQL");
  // clang-format on

  // Querying with `=` is about 2x faster than `like`, so don't use
  // `like` unless it's actually required.
  if (!catalog.empty()) {
    if (catalog.find('%') != std::string::npos) {
      query += std::string("AND table_cat LIKE '" + catalog + "'\n");
    } else {
      query += std::string("AND table_cat = '" + catalog + "'\n");
    }
  }
  if (!schema.empty()) {
    if (schema.find('%') != std::string::npos) {
      query += std::string("AND table_schem LIKE '" + schema + "'\n");
    } else {
      query += std::string("AND table_schem = '" + schema + "'\n");
    }
  }
  if (!tableName.empty()) {
    if (tableName.find('%') != std::string::npos) {
      query += std::string("AND table_name LIKE '" + tableName + "'\n");
    } else {
      query += std::string("AND table_name = '" + tableName + "'\n");
    }
  }
  if (!columnName.empty()) {
    if (columnName.find('%') != std::string::npos) {
      query += std::string("AND column_name LIKE '" + columnName + "'\n");
    } else {
      query += std::string("AND column_name = '" + columnName + "'\n");
    }
  }

  // Mandatory order-by clause
  query = query + std::string("\
   ORDER BY \
        table_cat, table_schem, table_name, ordinal_position \
   ");

  return query;
}

SQLRETURN SQL_API
SQLColumns(SQLHSTMT StatementHandle,
           _In_reads_opt_(NameLength1) SQLCHAR* CatalogNameChars,
           SQLSMALLINT NameLength1,
           _In_reads_opt_(NameLength2) SQLCHAR* SchemaNameChars,
           SQLSMALLINT NameLength2,
           _In_reads_opt_(NameLength3) SQLCHAR* TableNameChars,
           SQLSMALLINT NameLength3,
           _In_reads_opt_(NameLength4) SQLCHAR* ColumnNameChars,
           SQLSMALLINT NameLength4) {
  WriteLog(LL_TRACE, "Entering SQLColumns");
  if (!StatementHandle) {
    WriteLog(LL_ERROR, "  ERROR: Invalid handle in SQLTables");
    return SQL_INVALID_HANDLE;
  }
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  std::string catalogName = stringFromChar(CatalogNameChars, NameLength1);
  std::string schemaName  = stringFromChar(SchemaNameChars, NameLength2);
  std::string tableName   = stringFromChar(TableNameChars, NameLength3);
  std::string columnName  = stringFromChar(ColumnNameChars, NameLength4);

  WriteLog(LL_TRACE, "  Requested catalog: " + catalogName);
  WriteLog(LL_TRACE, "  Requested schema: " + schemaName);
  WriteLog(LL_TRACE, "  Requested table: " + tableName);
  WriteLog(LL_TRACE, "  Requested columnName: " + columnName);

  std::string query =
      constructColumnQuery(catalogName, schemaName, tableName, columnName);
  statement->trinoQuery->setQuery(query);
  statement->trinoQuery->post();
  statement->executed = true;
  return SQL_SUCCESS;
}
