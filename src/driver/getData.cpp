#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

#include "../trinoAPIWrapper/columnDescription.hpp"
#include "../util/rowToBuffer.hpp"
#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

using json = nlohmann::json;

SQLRETURN SQL_API SQLGetData(SQLHSTMT StatementHandle,
                             SQLUSMALLINT columnNumber,
                             SQLSMALLINT cDataType,
                             _Out_writes_opt_(_Inexpressible_(bufferLength))
                                 SQLPOINTER buffer,
                             SQLLEN bufferLength,
                             _Out_opt_ SQLLEN* strLen_or_IndPtr) {
  /*
  The cDataType is an indicator of the type of buffer that the application
  has resereved to receive the data from the driver. That may be different
  than the type of the column in the database.

  The columnNumber can be zero (0), which corresponds to the bookmark column.
  Column number one (1) is the first column of data returned by the
  query itself.

  bufferLength and strLen_or_IndPtr seems similar at first glance, but they're
  different in an important way. bufferLength is the size of the buffer
  the driver can use to copy data into. It's an input to this function.
  strLen_or_IndPtr is, in the case of a varchar, an output column that indicates
  the total length of data available in the source, which may be more than
  the size of the buffer provided by the application.
  */
  WriteLog(LL_TRACE, "Entering SQLGetData");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  const std::vector<ColumnDescription>& columnDescriptions =
      statement->trinoQuery->getColumnDescriptions();

  // Make sure the column number is within the bounds of the columns
  // returned by the query.
  if (columnNumber < 1 || columnNumber > columnDescriptions.size()) {
    WriteLog(LL_ERROR, "  ERROR: SQLGetData - Column out of bounds");
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = SQL_NULL_DATA;
    }
    if (buffer) {
      buffer = nullptr;
    }
    return SQL_ERROR;
  }

  Descriptor* rowDescriptor = statement->getRowDescriptor();
  const ColumnDescription& thisColumnDescription =
      columnDescriptions.at(columnNumber - 1);
  // The bookmark column is column 0
  DescriptorField descriptorField = rowDescriptor->getField(columnNumber);
  SQLSMALLINT odbcDataType        = descriptorField.odbcDataType;

  SQLLEN fetchedPosition = statement->getFetchedPosition();
  const json& rowData = statement->trinoQuery->getRowAtIndex(fetchedPosition);

  // Handle null data
  if (rowData[columnNumber - 1].is_null()) {
    if (strLen_or_IndPtr) {
      *strLen_or_IndPtr = SQL_NULL_DATA;
    }
    if (buffer) {
      buffer = nullptr;
    }
    WriteLog(LL_TRACE, "  Returning NULL for requested data");
    return SQL_SUCCESS;
  }

  if (getLogLevel() <= LL_TRACE) {
    WriteLog(LL_TRACE,
             "  Getting data for column: " + thisColumnDescription.getName());
    WriteLog(LL_TRACE, "  CDataType is: " + std::to_string(cDataType));
  }

  ColumnToBufferStatus status = columnToBuffer(cDataType,
                                               odbcDataType,
                                               rowData,
                                               columnNumber,
                                               buffer,
                                               bufferLength,
                                               strLen_or_IndPtr,
                                               descriptorField.precision,
                                               descriptorField.scale);

  // If the client doesn't reserve enough buffer space to hold the variable
  // length data returned, we need to right-truncate it to fit the buffer
  // and return a different status to warn the client.
  if (status.isVariableLength and strLen_or_IndPtr and
      *strLen_or_IndPtr > bufferLength) {
    ErrorInfo errorInfo = ErrorInfo("String data, right truncated", "01004");
    statement->setError(errorInfo);
    return SQL_SUCCESS_WITH_INFO;
  } else {
    return SQL_SUCCESS;
  }
}
