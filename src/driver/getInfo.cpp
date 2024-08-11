#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>

#include "../util/valuePtrHelper.hpp"
#include "../util/writeLog.hpp"
#include "handles/connHandle.hpp"

_Success_(return == SQL_SUCCESS) SQLRETURN SQL_API
    SQLGetInfo(SQLHDBC ConnectionHandle,
               SQLUSMALLINT InfoType,
               _Out_writes_bytes_opt_(BufferLength) SQLPOINTER InfoValue,
               SQLSMALLINT BufferLength,
               _Out_opt_ SQLSMALLINT* StringLengthPtr) {
  Connection* connection = reinterpret_cast<Connection*>(ConnectionHandle);
  WriteLog(LL_TRACE, "Entering SQLGetInfo");
  WriteLog(LL_TRACE,
           "  Requesting information type: " + std::to_string(InfoType));

  if (InfoValue == nullptr) {
    WriteLog(LL_ERROR, "  ERROR: Exiting SQLGetInfo - InfoValue is null");
    return SQL_ERROR;
  }

  switch (InfoType) {
    case SQL_MAX_DRIVER_CONNECTIONS: { // 0
      // Zero represents no specified limit or an unknown limit.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_MAX_CONCURRENT_ACTIVITIES: { // 1
      // Zero represents no specified limit or an unknown limit.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_DRIVER_NAME: { // 6
      writeNullTermStringToPtr(InfoValue, "TrinoODBC", StringLengthPtr);
      break;
    }
    case SQL_DRIVER_VER: { // 7
      writeNullTermStringToPtr(InfoValue, "00.00.0001", StringLengthPtr);
      break;
    }
    case SQL_SEARCH_PATTERN_ESCAPE: { // 14
      // Trino supports escapes in search patterns:
      // x LIKE y ESCAPE '/'
      // but that's not a hard-coded constant like this driver is expecting.
      // So we return empty string = no escaping supported.
      writeNullTermStringToPtr(InfoValue, "", StringLengthPtr);
      break;
    }
    case SQL_DBMS_NAME: { // 17
      // What's the name of this DBMS? Trino!
      writeNullTermStringToPtr(InfoValue, "Trino", StringLengthPtr);
      break;
    }
    case SQL_DBMS_VER: { // 18
      // What's the version of this DBMS? We have to query to find out.
      // It must be of the form ##.##.####, but Trino doesn't do that.
      // We're allow to append a product-specific version, so we will
      // put the actual version there.
      std::string rawServerVersion = connection->getServerVersion();
      std::string versionString    = "00.00.0001 " + rawServerVersion;
      writeNullTermStringToPtr(InfoValue, versionString, StringLengthPtr);
      break;
    }
    case SQL_ACCESSIBLE_TABLES: { // 19
      // If a table is returned by SQLTables, is the user guaranteed
      // to have SELECT privileges on it? No
      writeNullTermStringToPtr(InfoValue, "N", StringLengthPtr);
      break;
    }
    case SQL_CONCAT_NULL_BEHAVIOR: { // 22
      // SELECT 'hello' || null
      // Trino returns NULL (SQL-92 compliant)
      *((SQLUSMALLINT*)InfoValue) = SQL_CB_NULL;
      break;
    }
    case SQL_CURSOR_COMMIT_BEHAVIOR: { // 23
      // TODO: What is the actual behavior?
      // PyODBC wants to know this. We don't
      // support transactions yet, so for now
      // it doesn't matter.
      *((SQLUSMALLINT*)InfoValue) = SQL_CB_DELETE;
      break;
    }
    case SQL_CURSOR_ROLLBACK_BEHAVIOR: { // 24
      // TODO: What is the actual behavior?
      // PyODBC wants to know this. We don't
      // support transactions yet, so for now
      // it doesn't matter.
      *((SQLUSMALLINT*)InfoValue) = SQL_CB_DELETE;
      break;
    }
    case SQL_IDENTIFIER_QUOTE_CHAR: { // 29
      // TODO: What is the actual behavior?
      // PyODBC wants to know this.
      writeNullTermStringToPtr(InfoValue, "\"", StringLengthPtr);
      break;
    }
    case SQL_MAX_SCHEMA_NAME_LEN: { // 32
      // What's the longest possible schema name?
      *((SQLUSMALLINT*)InfoValue) = 128;
      break;
    }
    case SQL_MAX_CATALOG_NAME_LEN: { // 34
      // What's the longest possible catalog name?
      *((SQLUSMALLINT*)InfoValue) = 128;
      break;
    }
    case SQL_MAX_TABLE_NAME_LEN: { // 35
      // What's the longest possible table name?
      *((SQLUSMALLINT*)InfoValue) = 128;
      break;
    }
    case SQL_OWNER_TERM: { // 39
      // What's the thing that owns tables called?
      // Trino calls that a "schema", (SQL-92 compliant).
      writeNullTermStringToPtr(InfoValue, "schema", StringLengthPtr);
      break;
    }
    case SQL_QUALIFIER_NAME_SEPARATOR: { // 41
      // What's the thing that separates catalogs, schemas, and tables called?
      // Trino uses ".", (SQL-92 compliant).
      writeNullTermStringToPtr(InfoValue, ".", StringLengthPtr);
      break;
    }
    case SQL_CATALOG_TERM: { // 42
      // What does trino call the "catalog" part of catalog.schema.table?
      // Trino uses "catalog", (SQL-92 compliant).
      writeNullTermStringToPtr(InfoValue, "catalog", StringLengthPtr);
      break;
    }
    case SQL_CONVERT_FUNCTIONS: { // 48
      // What convert functions does Trino support?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
          SQL_FN_CVT_CAST |
          /* SQL_FN_CVT_CONVERT | */
          0;
      // clang-format on
      break;
    }
    case SQL_NUMERIC_FUNCTIONS: { // 49
      // What numeric functions does Trino support?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
          SQL_FN_NUM_ABS |
          SQL_FN_NUM_ACOS |
          SQL_FN_NUM_ASIN |
          SQL_FN_NUM_ATAN |
          SQL_FN_NUM_ATAN2 |
          SQL_FN_NUM_CEILING |
          SQL_FN_NUM_COS |
          /* SQL_FN_NUM_COT | */
          SQL_FN_NUM_DEGREES |
          SQL_FN_NUM_EXP |
          SQL_FN_NUM_FLOOR |
          SQL_FN_NUM_LOG |
          SQL_FN_NUM_LOG10 |
          SQL_FN_NUM_MOD |
          SQL_FN_NUM_PI |
          SQL_FN_NUM_POWER |
          SQL_FN_NUM_RADIANS |
          SQL_FN_NUM_RAND |
          SQL_FN_NUM_ROUND |
          SQL_FN_NUM_SIGN |
          SQL_FN_NUM_SIN |
          SQL_FN_NUM_SQRT |
          SQL_FN_NUM_TAN |
          SQL_FN_NUM_TRUNCATE |
          0;
      // clang-format on
      break;
    }
    case SQL_STRING_FUNCTIONS: { // 50
      // What string functions does Trino support?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
          /* SQL_FN_STR_ASCII | */
          /* SQL_FN_STR_BIT_LENGTH | */
          /* SQL_FN_STR_CHAR | */
          /* SQL_FN_STR_CHAR_LENGTH | */
          /* SQL_FN_STR_CHARACTER_LENGTH | */
          SQL_FN_STR_CONCAT |
          /* SQL_FN_STR_DIFFERENCE | */
          /* SQL_FN_STR_INSERT | */
          /* SQL_FN_STR_LCASE */
          /* SQL_FN_STR_LEFT */
          SQL_FN_STR_LENGTH |
          /* SQL_FN_STR_LOCATE */
          SQL_FN_STR_LTRIM |
          /* SQL_FN_STR_OCTET_LENGTH | */
          SQL_FN_STR_POSITION |
          /* SQL_FN_STR_REPEAT | */
          /* SQL_FN_STR_REPLACE | */
          /* SQL_FN_STR_RIGHT | */
          SQL_FN_STR_RTRIM |
          SQL_FN_STR_SOUNDEX |
          /* SQL_FN_STR_SPACE | */
          SQL_FN_STR_SUBSTRING |
          /* SQL_FN_STR_UCASE */
          0;
      // clang-format on
      break;
    }
    case SQL_SYSTEM_FUNCTIONS: { // 51
      // What system functions does Trino support?
      // None of them...
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
          /* SQL_FN_SYS_DBNAME | */
          /* SQL_FN_SYS_IFNULL | */
          /* SQL_FN_SYS_USERNAME | */
          0;
      // clang-format on
      break;
    }
    case SQL_TIMEDATE_FUNCTIONS: { // 52
      // What datetime functions does Trino support?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_FN_TD_CURRENT_DATE |
        SQL_FN_TD_CURRENT_TIME |
        SQL_FN_TD_CURRENT_TIMESTAMP |
        /* SQL_FN_TD_CURDATE | */
        /* SQL_FN_TD_CURTIME | */
        /* SQL_FN_TD_DAYNAME | */
        /* SQL_FN_TD_DAYOFMONTH | // (Trino has day_of_month) */
        /* SQL_FN_TD_DAYOFWEEK | // (Trino has day_of_week) */
        /* SQL_FN_TD_DAYOFYEAR | // (Trino has day_of_year) */
        SQL_FN_TD_EXTRACT |
        SQL_FN_TD_HOUR |
        SQL_FN_TD_MINUTE |
        SQL_FN_TD_MONTH |
        /* SQL_FN_TD_MONTHNAME | */
        SQL_FN_TD_NOW |
        SQL_FN_TD_QUARTER |
        SQL_FN_TD_SECOND |
        /* SQL_FN_TD_TIMESTAMPADD | */
        /* SQL_FN_TD_TIMESTAMPDIFF | */
        SQL_FN_TD_WEEK |
        SQL_FN_TD_YEAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_BIGINT: { // 53
      // What conversions does trino support for BIGINTs?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_BINARY: { // 54
      // What conversions does trino support for VARBINARY?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_VARBINARY |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_BIT: { // 55
      // What conversions does trino support for BIT?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_CHAR: { // 56
      // What conversions does trino support for CHAR?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_CHAR |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_DATE: { // 57
      // What conversions does trino support for DATE types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_DATE |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_TIMESTAMP |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_DECIMAL: { // 58
      // What conversions does trino support for decimal types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_DOUBLE: { // 59
      // What conversions does trino support for DOUBLE types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_FLOAT: { // 60
      // What conversions does trino support for FLOAT types?
      // Trino uses REAL, not FLOAT, so nothing is supported.
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_INTEGER: { // 61
      // What conversions does trino support for INTEGER types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_LONGVARCHAR: { // 62
      // What conversions does trino support for VARCHAR types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_CHAR |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_NUMERIC: { // 63
      // What conversions does trino support for NUMERIC types?
      // Trino does not support NUMERIC, so none.
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_REAL: { // 64
      // What conversions does trino support for REAL (float32) types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_SMALLINT: { // 65
      // What conversions does trino support for SMALLINT types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_TIME: { // 66
      // What conversions does trino support for TIME types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_DATE |
        SQL_CVT_TIME |
        SQL_CVT_TIMESTAMP |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_TIMESTAMP: { // 67
      // What conversions does trino support for TIMESTAMP types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_DATE |
        SQL_CVT_TIME |
        SQL_CVT_TIMESTAMP |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_TINYINT: { // 68
      // What conversions does trino support for TINYINT types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_BIGINT |
        SQL_CVT_BIT |
        SQL_CVT_DECIMAL |
        SQL_CVT_DOUBLE |
        SQL_CVT_INTEGER |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_REAL |
        SQL_CVT_SMALLINT |
        SQL_CVT_TINYINT |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_VARBINARY: { // 69
      // What conversions does trino support for VARBINARY types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_VARBINARY |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_VARCHAR: { // 70
      // What conversions does trino support for VARCHAR types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_CHAR |
        SQL_CVT_LONGVARCHAR |
        SQL_CVT_VARCHAR |
        0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_LONGVARBINARY: { // 71
      // What conversions does trino support for LONGVARBINARY types?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_CVT_LONGVARBINARY |
        SQL_CVT_VARBINARY |
        0;
      // clang-format on
      break;
    }
    case SQL_DRIVER_ODBC_VER: { // 77
      // We'll target the latest version of ODBC
      writeNullTermStringToPtr(InfoValue, "03.80", StringLengthPtr);
      break;
    }
    case SQL_GETDATA_EXTENSIONS: { // 81
      // What conventions that our SQLGetData implementation support.
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
        SQL_GD_ANY_COLUMN |
        SQL_GD_ANY_ORDER |
        /* SQL_GD_BLOCK | */
        SQL_GD_BOUND |
        /* SQL_GD_OUTPUT_PARAMS | */
        0;
      // clang-format on
      break;
    }
    case SQL_COLUMN_ALIAS: { // 87
      // Does trino support column aliases? Yeah.
      writeNullTermStringToPtr(InfoValue, "Y", StringLengthPtr);
      break;
    }
    case SQL_GROUP_BY: { // 88
      // Describe group-by semantics and relationship support in Trino
      // clang-format off
      *((SQLUSMALLINT*)InfoValue) = 0 |
          /* SQL_GB_COLLATE | (Not supported) */
          /* SQL_GB_NOT_SUPPORTED | (It _is_ supported) */
          SQL_GB_GROUP_BY_EQUALS_SELECT |
          /* SQL_GB_GROUP_BY_CONTAINS_SELECT | */
          /* SQL_GB_NO_RELATION | */
          0;
      // clang-format on
      break;
    }
    case SQL_ORDER_BY_COLUMNS_IN_SELECT: { // 90
      // Do columns in the order by clause need to be in the select list?
      // Nope
      writeNullTermStringToPtr(InfoValue, "N", StringLengthPtr);
      break;
    }
    case SQL_SCHEMA_USAGE: { // 91
      // In what context can schemas be used in query statements?
      *((SQLUINTEGER*)InfoValue) = 0 | SQL_SU_DML_STATEMENTS |
                                   SQL_SU_PROCEDURE_INVOCATION |
                                   SQL_SU_TABLE_DEFINITION |
                                   /* SQL_SU_INDEX_DEFINITION | (I don't think
                                      trino can create indices */
                                   SQL_SU_PRIVILEGE_DEFINITION | 0;
      break;
    }
    case SQL_CATALOG_USAGE: { // 92
      // In what context can catalogs be used in query statements?
      *((SQLUINTEGER*)InfoValue) = 0 | SQL_CU_DML_STATEMENTS |
                                   SQL_CU_PROCEDURE_INVOCATION |
                                   SQL_CU_TABLE_DEFINITION |
                                   /* SQL_CU_INDEX_DEFINITION | (I don't think
                                      trino can create indices */
                                   SQL_CU_PRIVILEGE_DEFINITION | 0;
      break;
    }
    case SQL_SPECIAL_CHARACTERS: { // 94
      // What special characters are allowed to be included in identifiers names
      // in Trino? Using these characters will require double-quoting identifier
      // names, but they're still allowed.
      writeNullTermStringToPtr(InfoValue, "$-@", StringLengthPtr);
      break;
    }
    case SQL_MAX_COLUMNS_IN_GROUP_BY: { // 97
      // How many columns can be in a group-by statement? Pratically unlimited.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_MAX_COLUMNS_IN_INDEX: { // 98
      // How many columns can be in a index?
      // None. Trino doesn't support indices.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_MAX_COLUMNS_IN_ORDER_BY: { // 99
      // How many columns can be in a order-by statement? Pratically unlimited.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_MAX_COLUMNS_IN_SELECT: { // 100
      // How many columns can be in a select statement? Pratically unlimited.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_MAX_COLUMNS_IN_TABLE: { // 101
      // How many columns can be in a table? Pratcically unlimited.
      *((SQLUSMALLINT*)InfoValue) = 0;
      break;
    }
    case SQL_TIMEDATE_ADD_INTERVALS: { // 109
      // Trino doesn't support the TIMESTAMPADD function,
      // so this bitfield is zeroed out.
      *((SQLUINTEGER*)InfoValue) = 0;
      break;
    }
    case SQL_TIMEDATE_DIFF_INTERVALS: { // 110
      // Trino doesn't support the TIMESTAMPDIFF function,
      // so this bitfield is zeroed out.
      *((SQLUINTEGER*)InfoValue) = 0;
      break;
    }
    case SQL_NEED_LONG_DATA_LEN: { // 111
      // Does trino need to know how long the data values
      // being sent are _before_ they are sent? No.
      writeNullTermStringToPtr(InfoValue, "N", StringLengthPtr);
      break;
    }
    case SQL_CATALOG_LOCATION: { // 114
      // Where does the catalog go in a fully qualified table name?
      // Trino puts it at the start. <catalog>.<schema>.<table>
      *((SQLUSMALLINT*)InfoValue) = SQL_CL_START;
      break;
    }
    case SQL_SQL_CONFORMANCE: { // 118
      // What level of SQL spec does Trino conform to? Let's guess the
      // lowest level for which there is a constant defined.
      *((SQLUINTEGER*)InfoValue) = SQL_SC_SQL92_ENTRY;
      break;
    }
    case SQL_CONVERT_WCHAR: { // 122
      // What conversions does trino support for WVARCHAR types?
      // None, I don't think trino supports wide varchars. UTF-8 all the way.
      *((SQLUINTEGER*)InfoValue) = 0 | 0;
      break;
    }
    case SQL_CONVERT_WLONGVARCHAR: { // 125
      // What conversions does trino support for WVARCHAR types?
      // None, I don't think trino supports wide varchars. UTF-8 all the way.
      *((SQLUINTEGER*)InfoValue) = 0 | 0;
      break;
    }
    case SQL_CONVERT_WVARCHAR: { // 126
      // What conversions does trino support for WVARCHAR types?
      // None, I don't think trino supports wide varchars. UTF-8 all the way.
      *((SQLUINTEGER*)InfoValue) = 0 | 0;
      break;
    }
    case SQL_ODBC_INTERFACE_CONFORMANCE: { // 152
      // How much of the ODBC interface spec does this driver implement?
      // Just the core level for now.
      *((SQLUINTEGER*)InfoValue) = SQL_OIC_CORE;
      break;
    }
    case SQL_SQL92_PREDICATES: { // 160
      // What predicates are supported in SELECT statements?
      // clang-format off
      *((SQLUINTEGER*)InfoValue) = 0 |
          SQL_SP_BETWEEN | // SELECT 1 BETWEEN 0 AND 2
          SQL_SP_COMPARISON | // There are a lot of comparison predictes. Let's just say yes.
          SQL_SP_EXISTS | // SELECT EXISTS(SELECT 1)
          SQL_SP_IN | // SELECT 1 IN (1, 2, 3)
          SQL_SP_ISNOTNULL | // SELECT 1 IS NOT NULL
          SQL_SP_ISNULL | // SELECT 1 IS NOT NULL
          SQL_SP_LIKE | // SELECT 'abc' LIKE 'a%'
          /*
          // I don't think any of these are implemented
          SQL_SP_MATCH_FULL (Full level)
          SQL_SP_MATCH_PARTIAL(Full level)
          SQL_SP_MATCH_UNIQUE_FULL (Full level)
          SQL_SP_MATCH_UNIQUE_PARTIAL (Full level)
          SQL_SP_OVERLAPS (FIPS Transitional level)
          SQL_SP_QUANTIFIED_COMPARISON (Entry level)
          SQL_SP_UNIQUE (Entry level)
          */
          0;
      // clang-format on
      break;
    }
    case SQL_SQL92_RELATIONAL_JOIN_OPERATORS: { // 161
      // What relational join operators are supported by Trino?
      *((SQLUINTEGER*)InfoValue) =
          0 |
          /* SQL_SRJO_CORRESPONDING_CLAUSE(Intermediate level) */
          SQL_SRJO_CROSS_JOIN |
          /* SQL_SRJO_EXCEPT_JOIN(Intermediate level) */
          SQL_SRJO_FULL_OUTER_JOIN | SQL_SRJO_INNER_JOIN |
          /* SQL_SRJO_INTERSECT_JOIN(Intermediate level) */
          SQL_SRJO_LEFT_OUTER_JOIN |
          /* SQL_SRJO_NATURAL_JOIN(FIPS Transitional level) */
          SQL_SRJO_RIGHT_OUTER_JOIN |
          /* SQL_SRJO_UNION_JOIN(Full level)  I don't think this is the UNION
             between queries*/
          0;
      // clang-format on
      break;
    }
    case SQL_SQL92_VALUE_EXPRESSIONS: { // 165
      // What value expressions are supported by Trino?
      *((SQLUINTEGER*)InfoValue) =
          0 | SQL_SVE_CASE | // CASE WHEN ... THEN ... END CASE is supported
          SQL_SVE_CAST |     // CAST(x AS <TYPE>) is supported
          SQL_SVE_COALESCE | // COALESCE(..., ..., [...]) is supported.
          SQL_SVE_NULLIF |   // NULLIF(value1, value2) is supported.
          0;
      // clang-format on
      break;
    }
    case SQL_AGGREGATE_FUNCTIONS: { // 169
      // What aggregate functions does Trino support?
      *((SQLUINTEGER*)InfoValue) =
          0 |
          /*
          all() is not an aggregate function in Trino. It does have that keyword
          though, as in SELECT 21 < ALL (VALUES 19, 20, 21).
          */
          /* SQL_AF_ALL | */
          SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_DISTINCT | SQL_AF_MAX |
          SQL_AF_MIN | SQL_AF_SUM | 0;
      // clang-format on
      break;
    }
    case SQL_CONVERT_GUID: { // 173
      // What conversions does trino support for GUID (UUID) types?
      *((SQLUINTEGER*)InfoValue) = 0 | SQL_CVT_GUID | SQL_CVT_LONGVARCHAR | 0;
      // clang-format on
      break;
    }
    case SQL_DESCRIBE_PARAMETER: { // 10002
      // Can parameters be described? Yes. Trino supports
      // the `DESCRIBE INPUT` statement.
      writeNullTermStringToPtr(InfoValue, "Y", StringLengthPtr);
      break;
    }
    case SQL_CATALOG_NAME: { // 10003
      // Does trino support catalogs? Yes.
      writeNullTermStringToPtr(InfoValue, "Y", StringLengthPtr);
      break;
    }
    case SQL_MAX_IDENTIFIER_LEN: { // 10005
      // How long can an identifier be? I don't know, but 128
      // is the "FIPS Intermediate" minimum, so we'll guess that.
      *((SQLUSMALLINT*)InfoValue) = 128;
      break;
    }
    case SQL_DTC_TRANSITION_COST: { // 1750
      // https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlconnect-function?view=sql-server-ver16#optimizing-connection-pooling-performance
      // TODO: Is enlistment expensive for this driver?
      *((SQLUINTEGER*)InfoValue) = 0;
      break;
    }
    case SQL_ASYNC_MODE: { // 10021
      // Explicitly say we don't support async stuff
      *((SQLUINTEGER*)InfoValue) = SQL_AM_NONE;
      break;
    }
    case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS: { // 10022
      // Explicitly say we don't support async stuff
      *((SQLUINTEGER*)InfoValue) = 0;
      break;
    }
    case SQL_ASYNC_DBC_FUNCTIONS: { // 10023
      // Explicitly say we don't support async stuff
      *((SQLUINTEGER*)InfoValue) = SQL_ASYNC_DBC_NOT_CAPABLE;
      break;
    }
    case SQL_ASYNC_NOTIFICATION: { // 10025
      // Explicitly say we don't support async stuff
      *((SQLUINTEGER*)InfoValue) = SQL_ASYNC_NOTIFICATION_NOT_CAPABLE;
      break;
    }
    // Handle other InfoType cases...
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: No info for requested parameter: " +
                   std::to_string(InfoType) + " - returning error code");
      std::string errorMessage =
          "Unknown InfoType to SQLGetInfo with id: " + std::to_string(InfoType);
      // HY091 = Invalid Descriptor Field Identifier.
      connection->setError(ErrorInfo(errorMessage, "HY091"));
      return SQL_ERROR;
    }
  }
  return SQL_SUCCESS;
};
