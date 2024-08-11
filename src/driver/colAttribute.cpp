#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>

#include "../util/valuePtrHelper.hpp"
#include "../util/writeLog.hpp"
#include "handles/statementHandle.hpp"
#include "mappings/typeMappings.hpp"

#pragma warning(push)
/*
The CharacterAttributePtr and NumericAttributePtr fields may
be unused if they aren't relevant to the requested attribute/column.
This is a documented expectation for these out parameters, so we'll
disable the warning about returning uninitialized memory in an out
parameter.
https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/sqlcolattribute-function
*/
#pragma warning(disable : 6101)

/*
 The signature of this function differs between 64-bit and 32-bit
 targets. For 32-bit, NumericAttributePtr is a SQLPOINTER, but for
 64-bit, its a SQLLEN*.
 */
#if defined(_WIN64)
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT StatementHandle,
                                  SQLUSMALLINT ColumnNumber,
                                  SQLUSMALLINT FieldIdentifier,
                                  _Out_writes_bytes_opt_(BufferLength)
                                      SQLPOINTER CharacterAttributePtr,
                                  SQLSMALLINT BufferLength,
                                  _Out_opt_ SQLSMALLINT* StringLengthPtr,
                                  _Out_opt_ SQLLEN* NumericAttributePtr) {
#else
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT StatementHandle,
                                  SQLUSMALLINT ColumnNumber,
                                  SQLUSMALLINT FieldIdentifier,
                                  _Out_writes_bytes_opt_(BufferLength)
                                      SQLPOINTER CharacterAttributePtr,
                                  SQLSMALLINT BufferLength,
                                  _Out_opt_ SQLSMALLINT* StringLengthPtr,
                                  _Out_opt_ SQLPOINTER NumericAttributePtr) {
#endif
#pragma warning(pop)
  WriteLog(LL_TRACE, "Entering SQLColAttribute");
  Statement* statement = reinterpret_cast<Statement*>(StatementHandle);

  Descriptor* ird            = statement->impRowDesc;
  DescriptorField columnInfo = ird->getField(ColumnNumber);

  switch (FieldIdentifier) {
    case SQL_DESC_CONCISE_TYPE: { // 2
      WriteLog(LL_TRACE, "  Getting SQL column type");
      SQLSMALLINT odbcTypeCode =
          TRINO_RAW_TYPE_TO_ODBC_TYPE_CODE[columnInfo.trinoRawTypeName];
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = odbcTypeCode;
      }
      break;
    }
    case SQL_DESC_UNSIGNED: { // 8
      // Trino does not support unsigned integer types, they're
      // always mapped to the next larger type. In the case of
      // unsigned int64s, they are mapped to decimals.
      WriteLog(LL_TRACE, "  Getting SQL column signed-nessness");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.isUnsigned;
      }
      break;
    }
    case SQL_COLUMN_TYPE_NAME: { // 14
      WriteLog(LL_TRACE, "  Getting SQL column name");
      std::string columnTypeName = columnInfo.trinoRawTypeName;
      writeNullTermStringToPtr(
          CharacterAttributePtr, columnTypeName, StringLengthPtr);
      break;
    }
    case SQL_DESC_NUM_PREC_RADIX: { // 32
      WriteLog(LL_TRACE, "  Getting SQL column attribute NumPrecRadix");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.numPrecRadix;
      }
      break;
    }
    case SQL_DESC_LENGTH: { // 1003
      WriteLog(LL_TRACE, "  Getting SQL column length");
      // How do we handle non-fixed length varchars?
      // For now, this defaults to SQL_NO_TOTAL (-4).
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.length;
      }
      break;
    }
    case SQL_DESC_PRECISION: { // 1005
      WriteLog(LL_TRACE, "  Getting SQL column precision");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.precision;
      }
      break;
    }
    case SQL_DESC_SCALE: { // 1006
      WriteLog(LL_TRACE, "  Getting SQL column scale");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.scale;
      }
      break;
    }
    case SQL_DESC_NULLABLE: { // 1008
      WriteLog(LL_TRACE, "  Getting SQL column null-abilty");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.nullable;
      }
      break;
    }
    case SQL_DESC_NAME: { // 1011
      WriteLog(LL_TRACE, "  Getting SQL column name");
      std::string columnName = columnInfo.columnName;
      writeNullTermStringToPtr(
          CharacterAttributePtr, columnName, StringLengthPtr);
      break;
    }
    case SQL_DESC_UNNAMED: { // 1012
      WriteLog(LL_TRACE, "  Getting SQL column named-ness");
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) = columnInfo.named;
      }
      break;
    }
    case SQL_DESC_OCTET_LENGTH: { // 1013
      WriteLog(LL_TRACE, "  Getting SQL column octet length");
      SQLULEN octetLength =
          TRINO_RAW_TYPE_TO_ODBC_SIZE_BYTES[columnInfo.trinoRawTypeName];
      if (NumericAttributePtr) {
        *((SQLINTEGER*)NumericAttributePtr) =
            static_cast<SQLINTEGER>(columnInfo.octetLength);
      }
      break;
    }
    default: {
      WriteLog(LL_ERROR,
               "  ERROR: Unhandled Column attribute type: " +
                   std::to_string(FieldIdentifier));
      return SQL_ERROR;
    }
  }

  return SQL_SUCCESS;
}
