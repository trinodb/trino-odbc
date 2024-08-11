#include "windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class ColumnToBufferStatus {
  public:
    bool isSuccess        = false;
    bool isVariableLength = false;
    ColumnToBufferStatus(bool isSuccess, bool isVariableLength);
};

ColumnToBufferStatus columnToBuffer(SQLSMALLINT cDataType,
                                    SQLSMALLINT odbcDataType,
                                    const json& rowData,
                                    SQLULEN columnNumber,
                                    void* buffer,
                                    SQLLEN bufferLength,
                                    SQLLEN* strLen_or_IndPtr,
                                    SQLCHAR precision,
                                    SQLCHAR scale);
