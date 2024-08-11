#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <map>
#include <nlohmann/json.hpp>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

using json = nlohmann::json;

class TypeInfo {
  private:
    std::string typeName;
    SQLSMALLINT dataType;
    SQLINTEGER columnSize;
    std::string literalPrefix;
    std::string literalSuffix;
    std::string createParams;
    SQLSMALLINT nullable;
    SQLSMALLINT caseSensitive;
    SQLSMALLINT searchable;
    SQLSMALLINT unsignedAttribute;
    SQLSMALLINT fixedPrecScale;
    SQLSMALLINT autoUniqueValue;
    std::string localTypeName;
    SQLSMALLINT minimumScale;
    SQLSMALLINT maximumScale;
    SQLSMALLINT sqlDataType;
    SQLSMALLINT sqlDatetimeSub;
    SQLINTEGER numPrecRadix;
    SQLSMALLINT intervalPrecision;

  public:
    TypeInfo(std::string typeName,
             SQLSMALLINT dataType,
             SQLINTEGER columnSize,
             std::string literalPrefix,
             std::string literalSuffix,
             std::string createParams,
             SQLSMALLINT nullable,
             SQLSMALLINT caseSensitive,
             SQLSMALLINT searchable,
             SQLSMALLINT unsignedAttribute,
             SQLSMALLINT fixedPrecScale,
             SQLSMALLINT autoUniqueValue,
             std::string localTypeName,
             SQLSMALLINT minimumScale,
             SQLSMALLINT maximumScale,
             SQLSMALLINT sqlDataType,
             SQLSMALLINT sqlDatetimeSub,
             SQLINTEGER numPrecRadix,
             SQLSMALLINT intervalPrecision) {
      this->typeName          = typeName;
      this->dataType          = dataType;
      this->columnSize        = columnSize;
      this->literalPrefix     = literalPrefix;
      this->literalSuffix     = literalSuffix;
      this->createParams      = createParams;
      this->nullable          = nullable;
      this->caseSensitive     = caseSensitive;
      this->searchable        = searchable;
      this->unsignedAttribute = unsignedAttribute;
      this->fixedPrecScale    = fixedPrecScale;
      this->autoUniqueValue   = autoUniqueValue;
      this->localTypeName     = localTypeName;
      this->minimumScale      = minimumScale;
      this->maximumScale      = maximumScale;
      this->sqlDataType       = sqlDataType;
      this->sqlDatetimeSub    = sqlDatetimeSub;
      this->numPrecRadix      = numPrecRadix;
      this->intervalPrecision = intervalPrecision;
    }

    json toJson() {
      return json::array(
          {typeName,
           dataType,
           columnSize,
           literalPrefix.empty() ? nullptr : json(literalPrefix),
           literalSuffix.empty() ? nullptr : json(literalSuffix),
           createParams.empty() ? nullptr : json(createParams),
           nullable == SQL_NULL_DATA ? nullptr : json(nullable),
           caseSensitive == SQL_NULL_DATA ? nullptr : json(caseSensitive),
           searchable == SQL_NULL_DATA ? nullptr : json(searchable),
           unsignedAttribute == SQL_NULL_DATA ? nullptr
                                              : json(unsignedAttribute),
           fixedPrecScale == SQL_NULL_DATA ? nullptr : json(fixedPrecScale),
           autoUniqueValue == SQL_NULL_DATA ? nullptr : json(autoUniqueValue),
           localTypeName.empty() ? nullptr : json(localTypeName),
           minimumScale == SQL_NULL_DATA ? nullptr : json(minimumScale),
           maximumScale == SQL_NULL_DATA ? nullptr : json(maximumScale),
           sqlDataType == SQL_NULL_DATA ? nullptr : json(sqlDataType),
           sqlDatetimeSub == SQL_NULL_DATA ? nullptr : json(sqlDatetimeSub),
           numPrecRadix == SQL_NULL_DATA ? nullptr : json(numPrecRadix),
           intervalPrecision == SQL_NULL_DATA ? nullptr
                                              : json(intervalPrecision)});
    }
};

// All returns from SQLGetTypeInfo results sets use the exact
// same column definitions. We can hard-code the Trino REST api
// response column signature, because it will be invariant to
// the type info requested. Only the data and row counts are
// different between types.
const json columnDescription = json::parse(R"???(
{
  "columns": [
    {"name": "TYPE_NAME",          "type": "varchar",  "typeSignature": { "rawType": "varchar",  "arguments": [{ "kind": "LONG", "value": 2147483647 }] }},
    {"name": "DATA_TYPE",          "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "COLUMN_SIZE",        "type": "integer",  "typeSignature": { "rawType": "integer",  "arguments": [] }},
    {"name": "LITERAL_PREFIX",     "type": "varchar",  "typeSignature": { "rawType": "varchar",  "arguments": [{ "kind": "LONG", "value": 2147483647 }] }},
    {"name": "LITERAL_SUFFIX",     "type": "varchar",  "typeSignature": { "rawType": "varchar",  "arguments": [{ "kind": "LONG", "value": 2147483647 }] }},
    {"name": "CREATE_PARAMS",      "type": "varchar",  "typeSignature": { "rawType": "varchar",  "arguments": [{ "kind": "LONG", "value": 2147483647 }] }},
    {"name": "NULLABLE",           "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "CASE_SENSITIVE",     "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "SEARCHABLE",         "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "UNSIGNED_ATTRIBUTE", "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "FIXED_PREC_SCALE",   "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "AUTO_UNIQUE_VALUE",  "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "LOCAL_TYPE_NAME",    "type": "varchar",  "typeSignature": { "rawType": "varchar",  "arguments": [{ "kind": "LONG", "value": 2147483647 }] }},
    {"name": "MINIMUM_SCALE",      "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "MAXIMUM_SCALE",      "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "SQL_DATA_TYPE",      "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "SQL_DATETIME_SUB",   "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }},
    {"name": "NUM_PREC_RADIX",     "type": "integer",  "typeSignature": { "rawType": "integer",  "arguments": [] }},
    {"name": "INTERVAL_PRECISION", "type": "smallint", "typeSignature": { "rawType": "smallint", "arguments": [] }}
  ]
}
)???");

// clang-format off
//                                      TYPE NAME,     DATA_TYPE, COLUMN_SIZE, LITERAL_PREFIX, LITERAL_SUFFIX, CREATE_PARAMS,     NULLABLE, CASE_SENSITIVE,     SEARCHABLE, UNSIGNED_ATTRIBUTE, FIXED_PREC_SCALE, AUTO_UNIQUE_VALUE, LOCAL_TYPE_NAME, MINIMUM_SCALE, MAXIMUM_SCALE, SQL_DATA_TYPE, SQL_DATETIME_SUB, NUM_PREC_RADIX, INTERVAL_PRECISION)
TypeInfo varcharTypeInfo   = TypeInfo(  "VARCHAR",   SQL_VARCHAR,  2147483647,             "",             "",      "length", SQL_NULLABLE,       SQL_TRUE, SQL_SEARCHABLE,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,       "VARCHAR", SQL_NULL_DATA, SQL_NULL_DATA,   SQL_VARCHAR,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo varbinaryTypeInfo = TypeInfo("VARBINARY", SQL_VARBINARY,  2147483647,            "x",             "",            "", SQL_NULLABLE,      SQL_FALSE,  SQL_PRED_NONE,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,     "VARBINARY", SQL_NULL_DATA, SQL_NULL_DATA, SQL_VARBINARY,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo bitTypeInfo       = TypeInfo(      "BIT",       SQL_BIT,           1,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,           "BIT",             0,             0,       SQL_BIT,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo tinyintTypeInfo   = TypeInfo(  "TINYINT",   SQL_TINYINT,           8,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,       "TINYINT",             0,             0,   SQL_TINYINT,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo smallintTypeInfo  = TypeInfo( "SMALLINT",  SQL_SMALLINT,          16,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,      "SMALLINT",             0,             0,  SQL_SMALLINT,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo integerTypeInfo   = TypeInfo(  "INTEGER",   SQL_INTEGER,          32,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,       "INTEGER",             0,             0,   SQL_INTEGER,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo bigintTypeInfo    = TypeInfo(   "BIGINT",    SQL_BIGINT,          64,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,        "BIGINT",             0,             0,    SQL_BIGINT,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo realTypeInfo      = TypeInfo(     "REAL",      SQL_REAL,          32,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,          "REAL",             0,             7,      SQL_REAL,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo doubleTypeInfo    = TypeInfo(   "DOUBLE",    SQL_DOUBLE,          64,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,        "DOUBLE",             0,            16,    SQL_DOUBLE,    SQL_NULL_DATA,              2,      SQL_NULL_DATA);
TypeInfo guidTypeInfo      = TypeInfo(     "GUID",      SQL_GUID,          36,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,          "GUID", SQL_NULL_DATA, SQL_NULL_DATA,      SQL_GUID,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo dateTypeInfo      = TypeInfo(     "DATE",      SQL_DATE,          10,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,          "DATE", SQL_NULL_DATA, SQL_NULL_DATA,      SQL_DATE,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo timeTypeInfo      = TypeInfo(     "TIME",      SQL_TIME,          21,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,          "TIME", SQL_NULL_DATA, SQL_NULL_DATA,      SQL_TIME,    SQL_NULL_DATA,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo timestampTypeInfo = TypeInfo("TIMESTAMP", SQL_TIMESTAMP,          23,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,        SQL_FALSE,     SQL_NULL_DATA,     "TIMESTAMP", SQL_NULL_DATA, SQL_NULL_DATA, SQL_TIMESTAMP,     SQL_DATETIME,  SQL_NULL_DATA,      SQL_NULL_DATA);
TypeInfo decimalTypeInfo   = TypeInfo(  "DECIMAL",   SQL_DECIMAL,          39,             "",             "",            "", SQL_NULLABLE,      SQL_FALSE, SQL_PRED_BASIC,      SQL_NULL_DATA,         SQL_TRUE,     SQL_NULL_DATA,       "DECIMAL",             0,            38,   SQL_DECIMAL,    SQL_NULL_DATA,             10,      SQL_NULL_DATA);
// clang-format on


SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT StatementHandle,
                                 SQLSMALLINT DataType) {
  WriteLog(LL_TRACE, "Entering SQLGetTypeInfo");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  WriteLog(LL_TRACE,
           "  Requesting type info for type code: " + std::to_string(DataType));

  json typeResponse = columnDescription;
  switch (DataType) {
    case SQL_ALL_TYPES: {
      // TODO: JSON TYPE
      // TODO: INTERVAL TYPE
      // TODO: ARRAY TYPE
      // TODO: MAP TYPE
      // TODO: TIME/TIMESTAMP WITH TIME ZONE?
      typeResponse["data"] = json::array({
          varcharTypeInfo.toJson(),
          varbinaryTypeInfo.toJson(),
          bitTypeInfo.toJson(),
          tinyintTypeInfo.toJson(),
          smallintTypeInfo.toJson(),
          integerTypeInfo.toJson(),
          bigintTypeInfo.toJson(),
          realTypeInfo.toJson(),
          doubleTypeInfo.toJson(),
          guidTypeInfo.toJson(),
          dateTypeInfo.toJson(),
          timeTypeInfo.toJson(),
          timestampTypeInfo.toJson(),
          decimalTypeInfo.toJson(),
      });
      break;
    }
    case SQL_WVARCHAR: {
      // TODO: Does WVARCHAR exist in Trino?
      // Is WVARCHAR any different than varchar?
      // I'm guessing Trino uses UTF-8, and not any wide varchars.
      typeResponse["data"] = json::array({varcharTypeInfo.toJson()});
      break;
    }
    case SQL_VARCHAR: {
      typeResponse["data"] = json::array({varcharTypeInfo.toJson()});
      break;
    }
    case SQL_VARBINARY: {
      typeResponse["data"] = json::array({varbinaryTypeInfo.toJson()});
      break;
    }
    case SQL_TIMESTAMP: {
      typeResponse["data"] = json::array({timestampTypeInfo.toJson()});
      break;
    }
    case SQL_TYPE_TIMESTAMP: {
      typeResponse["data"] = json::array({timestampTypeInfo.toJson()});
      break;
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: SQLGetTypeInfo returning failure for type id: " +
                   std::to_string(DataType));
      return SQL_ERROR;
    }
  }
  statement->trinoQuery->sideloadResponse(typeResponse);
  WriteLog(LL_TRACE,
           "  SQLGetTypeInfo returning success for type id: " +
               std::to_string(DataType));
  return SQL_SUCCESS;
}
