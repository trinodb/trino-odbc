#include "statementHandle.hpp"
#include <functional>
#include <string>

#include "../mappings/typeMappings.hpp"

#include "../../util/writeLog.hpp"

void Statement::columnsChangedCallback(TrinoQuery* trinoQuery) {
  /*
  It's important to update the row descriptors whenever the query returns
  information about the column contents.
  */
  Descriptor* rowDescriptor = this->getRowDescriptor();
  // Use 1-based indexing, since column 0 is used to hold the bookmark data.
  int i = 1;
  for (const ColumnDescription& colDescription :
       trinoQuery->getColumnDescriptions()) {
    DescriptorField field           = rowDescriptor->getField(i);
    const std::string& trinoRawType = colDescription.getRawType();
    field.columnName                = colDescription.getName();
    field.trinoRawTypeName          = trinoRawType;
    // TODO: How do we find out what fields are actually nullable from Trino?
    field.nullable = true;
    if (colDescription.getRawType() == "decimal") {
      // Decimal precision and scale are the values of the first and second
      // type arguments. Decimal types use a numerical precision radix of 10,
      // but that is set by the TRINO_RAW_TYPE_TO_NUM_PREC_RADIX lookup.
      field.precision =
          colDescription.getTypeArguments()[0]["value"].get<SQLCHAR>();
      field.scale =
          colDescription.getTypeArguments()[1]["value"].get<SQLCHAR>();
    }
    // Not all types have precision, but if they do, set it.
    if (TRINO_RAW_TYPE_TO_PRECISION.contains(trinoRawType)) {
      field.precision = TRINO_RAW_TYPE_TO_PRECISION.at(trinoRawType);
    }
    // Not all types have a numerical precision radix, but if they do, set it.
    if (TRINO_RAW_TYPE_TO_NUM_PREC_RADIX.contains(trinoRawType)) {
      field.numPrecRadix = TRINO_RAW_TYPE_TO_NUM_PREC_RADIX.at(trinoRawType);
    }
    try {
      // This is always true for integral types! Trino doesn't support unsigned
      // integral types. It internally converts unsigned types to the next
      // larger signed integral type. uint64s are turned to decimals, which
      // finishes the job.
      field.isUnsigned = TRINO_RAW_TYPE_TO_UNSIGNED.at(trinoRawType);
    } catch (const std::exception& ex) {
      WriteLog(LL_ERROR,
               "ERROR: Key " + trinoRawType +
                   " not found in unsigned lookup: " + ex.what());
    }
    try {
      field.odbcDataType = TRINO_RAW_TYPE_TO_ODBC_TYPE_CODE.at(trinoRawType);
    } catch (const std::exception& ex) {
      WriteLog(LL_ERROR,
               "ERROR: Key " + trinoRawType +
                   " not found in type code lookup: " + ex.what());
    }
    try {
      field.octetLength = TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES.at(trinoRawType);
    } catch (const std::exception& ex) {
      WriteLog(LL_ERROR,
               "ERROR: Key " + trinoRawType +
                   " not found in type code lookup: " + ex.what());
    }
    this->getRowDescriptor()->setField(i, field);
    i++;
  }
}

Statement::Statement(ConnectionConfig* connectionConfig) {
  this->trinoQuery   = new TrinoQuery(connectionConfig);
  this->impParamDesc = new Descriptor();
  this->impRowDesc   = new Descriptor();
  // Application descriptors are managed by the application,
  // and thus the driver is not responsible for allocating
  // them or managing their memory.
  this->appParamDesc = this->impParamDesc;
  this->appRowDesc   = this->impRowDesc;

  // Register a callback for times when column data has changed.
  this->trinoQuery->registerColumnDataChangeCallback(std::bind(
      &Statement::columnsChangedCallback, this, std::placeholders::_1));
}

Statement::~Statement() {
  delete this->trinoQuery;
  // We can free memory for the implementation descriptors,
  // but should not free the application descriptors because
  // those are allocated by and managed by the application.
  delete this->impParamDesc;
  delete this->impRowDesc;
}

/*
Reset gets this statement ready to be used again.
*/
void Statement::reset() {
  this->executed              = false;
  this->fetchExecuteConfirmed = false;
  this->fetchedPosition       = -1;
  this->trinoQuery->reset();
  this->impParamDesc->reset();
  this->impRowDesc->reset();
}

/*
Terminate stops an in-flight query immediately. It also
gets called just before freeing the statement handle as
a saftey mechanism to make sure any in-flight queries
aren't left hanging after the statement is freed.
*/
void Statement::terminate() {
  this->trinoQuery->terminate();
}

/*
If the application provides their own descriptor,
we will ignore the default one we are providing
within the driver.
*/
Descriptor* Statement::getRowDescriptor() {
  if (this->appRowDesc) {
    return this->appRowDesc;
  } else {
    return this->impRowDesc;
  }
}

Descriptor* Statement::getParamDescriptor() {
  /*
  If the application provides their own descriptor,
  we will ignore the default one we are providing
  within the driver.
  */
  if (this->appParamDesc) {
    return this->appParamDesc;
  } else {
    return this->impParamDesc;
  }
}

SQLLEN Statement::getFetchedPosition() {
  return this->fetchedPosition;
}

void Statement::setFetchedPosition(SQLLEN pos) {
  if (this->impRowDesc->Field_RowsProcessedPtr) {
    *(this->impRowDesc->Field_RowsProcessedPtr) = pos;
  }
  this->fetchedPosition = pos;
}

void Statement::setError(ErrorInfo errorInfo) {
  this->errorInfo = errorInfo;
}

ErrorInfo Statement::getError() {
  return this->errorInfo;
}
