#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../util/writeLog.hpp"
#include "handles/descriptorHandle.hpp"

SQLRETURN SQL_API SQLGetDescField(
    SQLHDESC DescriptorHandle,
    SQLSMALLINT RecNumber,
    SQLSMALLINT FieldIdentifier,
    _Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER Value,
    SQLINTEGER BufferLength,
    _Out_opt_ SQLINTEGER* StringLength) {
  Descriptor* descriptor = reinterpret_cast<Descriptor*>(DescriptorHandle);
  WriteLog(LL_TRACE, "Entering SQLGetDescField");
  WriteLog(LL_TRACE,
           "  Descriptor handle is :" +
               std::to_string((uintptr_t)(void**)descriptor));
  WriteLog(LL_TRACE,
           "  Requesting Descriptor Record: " + std::to_string(RecNumber));
  WriteLog(LL_TRACE,
           "  Requesting Field Identifier: " + std::to_string(FieldIdentifier));
  if (!Value) {
    WriteLog(LL_ERROR, "  ERROR: Invalid value pointer");
    return SQL_ERROR;
  }
  switch (FieldIdentifier) {
    case SQL_DESC_ARRAY_SIZE: { // 20
      *((SQLULEN*)Value) = descriptor->Field_ArraySize;
      break;
    }
    case SQL_DESC_ARRAY_STATUS_PTR: { // 21
      *((SQLUSMALLINT**)Value) = descriptor->Field_ArrayStatusPtr;
      break;
    }
    case SQL_DESC_BIND_OFFSET_PTR: { // 24
      *((SQLLEN**)Value) = descriptor->Field_BindOffsetPtr;
      break;
    }
    case SQL_DESC_BIND_TYPE: { // 25
      *((SQLUINTEGER*)Value) = descriptor->Field_BindType;
      break;
    }
    case SQL_DESC_ROWS_PROCESSED_PTR: { // 34
      *((SQLULEN**)Value) = descriptor->Field_RowsProcessedPtr;
      break;
    }
    case SQL_DESC_COUNT: { // 1001
      *((SQLSMALLINT*)Value) = descriptor->Field_Count;
      break;
    }
    case SQL_DESC_ALLOC_TYPE: { // 1099
      *((SQLSMALLINT*)Value) = descriptor->Field_AllocType;
      break;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unimplemented descriptor field requested");
      return SQL_ERROR;
    }
  }
  return SQL_SUCCESS;
}
