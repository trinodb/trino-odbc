#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>

#include "../util/stringFromChar.hpp"
#include "../util/stringSplitAndTrim.hpp"
#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

std::string ALL_CATALOGS_QUERY = R"SQL(
    SELECT
        table_cat,
        CAST(NULL AS VARCHAR) AS table_schem,
        CAST(NULL AS VARCHAR) AS table_name,
        CAST(NULL AS VARCHAR) AS table_type,
        CAST(NULL AS VARCHAR) AS remarks
    FROM system.jdbc.catalogs
)SQL";

std::string ALL_SCHEMAS_QUERY = R"SQL(
    SELECT
        CAST(NULL AS VARCHAR) AS table_cat,
        table_schem,
        CAST(NULL AS VARCHAR) AS table_name,
        CAST(NULL AS VARCHAR) AS table_type,
        CAST(NULL AS VARCHAR) AS remarks
  FROM system.jdbc.schemas
)SQL";

std::string ALL_TABLE_TYPES_QUERY = R"SQL(
  SELECT
      CAST(NULL AS VARCHAR) AS table_cat,
      CAST(NULL AS VARCHAR) AS table_schem,
      CAST(NULL AS VARCHAR) AS table_name,
      table_type,
      CAST(NULL AS VARCHAR) AS remarks
  FROM system.jdbc.table_types
)SQL";

std::string constructTableQuery(std::string catalog,
                                std::string schema,
                                std::string tableName,
                                std::string tableType) {
  std::string query = std::string(R"SQL(
    SELECT
        table_cat,
        table_schem,
        table_name,
        table_type,
        remarks
    FROM
        system.jdbc.tables
    WHERE 1 = 1
  )SQL");
  // Profiling shows that using `=` is faster than using `like'
  // so we should use `=` if it's possible to do so.
  if (catalog.find('%') != std::string::npos) {
    query += "AND table_cat LIKE '" + catalog + "'\n";
  } else {
    query += "AND table_cat = '" + catalog + "'\n";
  }
  if (schema.find('%') != std::string::npos) {
    query += "AND table_schem LIKE '" + schema + "'\n";
  } else {
    query += "AND table_schem = '" + schema + "'\n";
  }
  if (tableName.find('%') != std::string::npos) {
    query += "AND table_name LIKE '" + tableName + "'\n";
  } else {
    query += "AND table_name = '" + tableName + "'\n";
  }
  if (tableType.empty() or tableType == "%") {
    query += "AND table_type LIKE '" + tableType + "'";
  } else {
    std::vector<std::string> tableTypesVec = stringSplitAndTrim(tableType, ',');
    if (!tableTypesVec.empty()) {
      // The app is requesting that we filter the types of tables returned
      query += "AND table_type IN (";
      for (std::vector<std::string>::size_type i = 0; i < tableTypesVec.size();
           i++) {
        // Insert the name of the table type enclosed in single quotes.
        query += "'" + tableTypesVec[i] + "'";
        // Insert a comma if it's not the last member of the list.
        if (i < tableTypesVec.size() - 1) {
          query += " ,";
        }
      }
      query += ")";
    }
  }

  return query;
}

SQLRETURN SQL_API SQLTables(SQLHSTMT StatementHandle,
                            _In_reads_opt_(NameLength1)
                                SQLCHAR* CatalogNameChars,
                            SQLSMALLINT NameLength1,
                            _In_reads_opt_(NameLength2)
                                SQLCHAR* SchemaNameChars,
                            SQLSMALLINT NameLength2,
                            _In_reads_opt_(NameLength3) SQLCHAR* TableNameChars,
                            SQLSMALLINT NameLength3,
                            _In_reads_opt_(NameLength4) SQLCHAR* TableTypeChars,
                            SQLSMALLINT NameLength4) {
  WriteLog(LL_TRACE, "Entering SQLTables");
  if (!StatementHandle) {
    WriteLog(LL_ERROR, "  ERROR: Invalid handle in SQLTables");
    return SQL_INVALID_HANDLE;
  }

  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  std::string catalogName = stringFromChar(CatalogNameChars, NameLength1);
  std::string schemaName  = stringFromChar(SchemaNameChars, NameLength2);
  std::string tableName   = stringFromChar(TableNameChars, NameLength3);
  std::string tableType   = stringFromChar(TableTypeChars, NameLength4);

  WriteLog(LL_TRACE, "  Requested catalog: " + catalogName);
  WriteLog(LL_TRACE, "  Requested schema: " + schemaName);
  WriteLog(LL_TRACE, "  Requested table: " + tableName);
  WriteLog(LL_TRACE, "  Requested table type: " + tableType);

  // Special cases to enable enumeration of catalogs, schemas, and table types.
  if (catalogName == SQL_ALL_CATALOGS and schemaName.empty() and
      tableName.empty() and tableType.empty()) {
    statement->trinoQuery->setQuery(ALL_CATALOGS_QUERY);
    statement->trinoQuery->post();
    statement->executed = true;
  } else if (schemaName == SQL_ALL_SCHEMAS and catalogName.empty() and
             tableName.empty()) {
    statement->trinoQuery->setQuery(ALL_SCHEMAS_QUERY);
    statement->trinoQuery->post();
    statement->executed = true;
  } else if (tableType == SQL_ALL_TABLE_TYPES and catalogName.empty() and
             schemaName.empty() and tableName.empty()) {
    statement->trinoQuery->setQuery(ALL_TABLE_TYPES_QUERY);
    statement->trinoQuery->post();
    statement->executed = true;
  } else {
    /*
    The docs make it sound like schema, tablename, and tabletype are all going
    to be search patterns. I'm not seeing that behavior from PowerBI though.
    It's passing empty strings in place of search patterns if it wants to match
    everything in a particular search field.
    */
    if (catalogName.empty()) {
      catalogName = std::string("%");
    }
    if (schemaName.empty()) {
      schemaName = std::string("%");
    }
    if (tableName.empty()) {
      tableName = std::string("%");
    }
    if (tableType.empty()) {
      tableType = std::string("%");
    }
    std::string query =
        constructTableQuery(catalogName, schemaName, tableName, tableType);
    WriteLog(LL_TRACE, "Final query is: " + query);
    statement->trinoQuery->setQuery(query);
    statement->trinoQuery->post();
    statement->executed = true;
  }

  return SQL_SUCCESS;
}
