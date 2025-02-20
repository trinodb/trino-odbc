#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"

SQLRETURN SQL_API SQLBindParameter(SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ParamNumber,
                                   SQLSMALLINT InputOutputType,
                                   SQLSMALLINT ValueType,
                                   SQLSMALLINT ParamType,
                                   SQLULEN ColumnSize,
                                   SQLSMALLINT DecimalDigits,
                                   SQLPOINTER ParamValuePtr,
                                   SQLLEN BufferLength,
                                   SQLLEN* StrLen_or_IndPtr) {

  /*
  This is called before SQLExecute when running a prepared statement.
  You bind the parameters to application variables ahead of time, so when
  SQLExecute is called, the parameters are automatically passed in the bound
  variables.

  InputOutputType refers to the type of the parameter. This particular one
  is referring to the SQL_DESC_PARAMETER_TYPE field in the Implementation
  Parameter Descriptor. This specifies how parameters are to be used.
  Options are:

  * SQL_PARAM_INPUT
  * SQL_PARAM_INPUT_OUTPUT
  * SQL_PARAM_OUTPUT
  * SQL_PARAM_INPUT_OUTPUT_STREAM
  * SQL_PARAM_OUTPUT_STREAM

  ValueType refers to the C data type of the buffer where this is allocated.
  That's not the same as the SQL type.

  ParamType refers to the SQL data type of the parameter.

  Note that TargetValuePtr may be a null pointer. If so, this effectively
  "unbinds" the column, since SQLFetch will treat descriptors with a null
  TargetValuePtr as not being bound.
  */
  WriteLog(LL_DEBUG, "Entering SQLBindParameter");
  WriteLog(LL_DEBUG, "  Param Number is: " + std::to_string(ParamNumber));
  WriteLog(LL_DEBUG, "  IO Type is: " + std::to_string(InputOutputType));
  WriteLog(LL_DEBUG, "  Value Type is: " + std::to_string(ValueType));
  WriteLog(LL_DEBUG, "  Param Type is: " + std::to_string(ParamType));
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);
  DescriptorField field =
      statement->getParamDescriptor()->getField(ParamNumber);
  field.odbcDataType         = ParamType;
  field.bufferCDataType      = ValueType;
  field.bufferPtr            = ParamValuePtr;
  field.bufferLength         = BufferLength;
  field.bufferStrLenOrIndPtr = StrLen_or_IndPtr;
  statement->getParamDescriptor()->setField(ParamNumber, field);

  WriteLog(LL_DEBUG, "  Parameter binding complete");
  return SQL_SUCCESS;
}
