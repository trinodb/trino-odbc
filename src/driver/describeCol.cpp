#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <map>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"
#include "mappings/typeMappings.hpp"

SQLULEN static inferODBCColumnSize(ColumnDescription description) {
  std::string rawType = description.getRawType();
  if (TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES.count(rawType)) {
    SQLULEN odbcSizeBytes = TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES[rawType];
    WriteLog(LL_TRACE,
             "  ODBC Type Size inferred as: " + std::to_string(odbcSizeBytes));
    return odbcSizeBytes;
  }
  if (rawType == "varchar") {
    // Varchars have an argument that specifies their length.
    return description.getTypeArguments()[0]["value"];
  }
  throw std::invalid_argument("Cannot determine size of column: " +
                              description.getName());
}

SQLSMALLINT static inferODBCTypeCode(ColumnDescription description) {
  std::string rawType      = description.getRawType();
  SQLSMALLINT odbcTypeCode = TRINO_RAW_TYPE_TO_ODBC_TYPE_CODE[rawType];
  WriteLog(LL_TRACE,
           "  ODBC Type Code inferred as: " + std::to_string(odbcTypeCode));
  return odbcTypeCode;
}

SQLRETURN SQL_API SQLDescribeCol(SQLHSTMT StatementHandle,
                                 SQLUSMALLINT ColumnNumber,
                                 _Out_writes_opt_(BufferLength)
                                     SQLCHAR* ColumnName,
                                 SQLSMALLINT BufferLength,
                                 _Out_opt_ SQLSMALLINT* NameLength,
                                 _Out_opt_ SQLSMALLINT* DataType,
                                 _Out_opt_ SQLULEN* ColumnSize,
                                 _Out_opt_ SQLSMALLINT* DecimalDigits,
                                 _Out_opt_ SQLSMALLINT* Nullable) {
  WriteLog(LL_TRACE, "Entering SQLDescribeCol");
  WriteLog(LL_TRACE, "  Column index: " + std::to_string(ColumnNumber));

  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  std::vector<ColumnDescription> columnDescriptions =
      statement->trinoQuery->getColumnDescriptions();

  ColumnDescription thisColumnDescription =
      columnDescriptions.at(ColumnNumber - 1);

  Descriptor* descriptorPtr       = statement->getRowDescriptor();
  DescriptorField descriptorField = descriptorPtr->getField(ColumnNumber);

  const std::string& columnNameString = thisColumnDescription.getName();
  SQLSMALLINT copyLength              = std::min<SQLSMALLINT>(
      BufferLength - 1, static_cast<SQLSMALLINT>(columnNameString.size()));

  if (ColumnName) {
    memcpy(ColumnName, columnNameString.c_str(), copyLength);
    ColumnName[copyLength] = '\0'; // Null-terminate.
  }
  if (NameLength) {
    *NameLength =
        static_cast<SQLSMALLINT>(thisColumnDescription.getName().size());
  }
  if (DataType) {
    *DataType = inferODBCTypeCode(thisColumnDescription);
  }
  if (ColumnSize) {
    // Unit is bytes for binary precision data.
    *ColumnSize = inferODBCColumnSize(thisColumnDescription);
  }
  if (Nullable) {
    // I don't know that trino provides a way to determine this based on
    // the underlying column metadata. It may be possible with additional
    // queries of metadata, but I don't think it's in the metadata
    // that comes with a normal SELECT statement. Assuming everything
    // is nullable seems like a reasonable default.
    *Nullable = SQL_NULLABLE;
  }
  if (DecimalDigits) {
    // The documentation says the value "0" should be set if the number of
    // digits cannot be determined or is not applicable. That's the default
    // value of the descriptorField struct's scale.
    *DecimalDigits = descriptorField.scale;
  }

  return SQL_SUCCESS;
}
