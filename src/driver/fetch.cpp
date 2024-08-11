#include "../util/windowsLean.hpp"
#include <sql.h>

#include <cstdint>
#include <string>

#include "../trinoAPIWrapper/trinoQuery.hpp"
#include "../util/rowToBuffer.hpp"
#include "../util/writeLog.hpp"
#include "handles/descriptorHandle.hpp"
#include "handles/statementHandle.hpp"

static void handleBoundColumns(Statement* statement) {
  /*
  Every time we fetch a row, we need to check if any of the data
  that was returned is from a column that has been bound to a
  buffer. If it has, we need to copy the data directly into
  the buffer before returning from the call to SQLFetch().
  */

  // First, make sure we have column information. We can't do anything with
  // bound columns until we know what columns we have.
  if (not statement->trinoQuery->hasColumnData()) {
    statement->trinoQuery->poll(UntilColumnsLoaded);
  }

  int16_t columnCount       = statement->trinoQuery->getColumnCount();
  Descriptor* rowDescriptor = statement->getRowDescriptor();
  SQLLEN fetchedPosition    = statement->getFetchedPosition();
  const json& rowData = statement->trinoQuery->getRowAtIndex(fetchedPosition);

  // Field indices start at 1 because index 0 is the "bookmark" column.
  for (auto i = 1; i <= columnCount; i++) {
    // It's safe to use `getFieldRef` here because we checked and confirmed
    // that we had loaded all the columns. They should have their descriptors
    // in place already.
    const DescriptorField& field = rowDescriptor->getFieldRef(i);

    // If the column isn't bound, there's nothing to be done.
    if (field.bufferPtr == nullptr) {
      continue;
    }

    SQLLEN* strLen_or_IndPtr = field.bufferStrLenOrIndPtr;
    void* buffer             = field.bufferPtr;
    SQLLEN bufferLength      = field.bufferLength;
    SQLSMALLINT cDataType    = field.bufferCDataType;
    SQLSMALLINT odbcDataType = field.odbcDataType;
    SQLULEN columnNumber     = i;

    // This is in a tight loop, so best to not even execute it
    // if there's a chance of skipping the calls to std::to_string.
    if (getLogLevel() <= LL_TRACE) {
      WriteLog(LL_TRACE, "  Bound column detected. Writing value");
      WriteLog(LL_TRACE, "  ODBC Type is: " + std::to_string(odbcDataType));
      WriteLog(LL_TRACE, "  C Type is: " + std::to_string(cDataType));
    }

    columnToBuffer(cDataType,
                   odbcDataType,
                   rowData,
                   columnNumber,
                   buffer,
                   bufferLength,
                   strLen_or_IndPtr,
                   field.precision,
                   field.scale);
  }
}

SQLRETURN SQL_API SQLFetch(SQLHSTMT StatementHandle) {
  WriteLog(LL_TRACE, "Entering SQLFetch");
  if (!StatementHandle) {
    WriteLog(LL_ERROR, "  ERROR: Invalid statement handle");
    return SQL_INVALID_HANDLE;
  }

  WriteLog(LL_TRACE, "  Getting Handles");
  Statement* statement   = reinterpret_cast<Statement*>(StatementHandle);
  TrinoQuery* trinoQuery = statement->trinoQuery;

  WriteLog(LL_TRACE, "  Checking row counts and completion");
  bool trinoQueryCompleted   = trinoQuery->getIsCompleted();
  int64_t trinoQueryRowCount = trinoQuery->getCurrentRowCount();
  SQLLEN fetchedPosition     = statement->getFetchedPosition();

  /*

  A call to Fetch can basically result in 4 possible flows of execution.
  This is a summary of what needs to happen.

  1.The query is done, no rows remain
    * Checkpoint the trino query to clear the row cache.
    * return SQL_NO_DATA;
  2. Rows remain available to advance to.
    * Advance by one row
    * return SQL_SUCCESS;
  3. The query is not done, but no rows were available to advance to.
    * Checkpoint the trino query to clear the row cache.
    * Poll for more data.
    * Start the fetch over again.
  4. Some kind of error - report and abort
    * return SQL_ERROR;

  */

  if (trinoQueryCompleted and fetchedPosition == (trinoQueryRowCount - 1)) {
    // Handle the case that the query has been completed
    // and there is no more data
    WriteLog(LL_TRACE, "  SQLFetch is indicating that no data remains");
    statement->trinoQuery->checkpointRowPosition(fetchedPosition);
    return SQL_NO_DATA;
  } else if (fetchedPosition < (trinoQueryRowCount - 1)) {
    // Handle the case that data is waiting to be read.
    WriteLog(LL_TRACE, "  There are more rows to read. Advancing row pointer.");
    statement->setFetchedPosition(fetchedPosition + 1);
    handleBoundColumns(statement);
    return SQL_SUCCESS;

  } else if (not trinoQueryCompleted) {
    // Handle the case that the query is not yet completed, but there's
    // also more data to read. This indicates we need to poll Trino to
    // obtain some more data. First, we will checkpoint the current
    // position to free any memory consumed by old rows.
    statement->trinoQuery->checkpointRowPosition(fetchedPosition);
    WriteLog(LL_TRACE, "  Trino query not completed. Polling until new data");
    // By default, the poll mode is UntilNewData.
    TrinoQueryPollMode pollMethod = statement->fetchPollMode;
    statement->trinoQuery->poll(pollMethod);
    WriteLog(LL_TRACE, "  Trino poll complete");
    int64_t newTrinoRowCount = trinoQuery->getCurrentRowCount();
    WriteLog(LL_TRACE, "  Got row count: " + std::to_string(newTrinoRowCount));

    return SQLFetch(StatementHandle);

  } else {
    WriteLog(LL_ERROR, "  ERROR: This should not be happening");
    WriteLog(LL_DEBUG,
             "  TrinoQueryCompleted: " + std::to_string(trinoQueryCompleted));
    WriteLog(LL_DEBUG, "  fetchedPosition: " + std::to_string(fetchedPosition));
    WriteLog(LL_DEBUG,
             "  trinoQueryRowCount: " + std::to_string(trinoQueryRowCount));
    return SQL_ERROR;
  }
};
