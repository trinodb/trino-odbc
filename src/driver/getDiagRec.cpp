#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "handles/connHandle.hpp"
#include "handles/descriptorHandle.hpp"
#include "handles/envHandle.hpp"
#include "handles/statementHandle.hpp"

#include "../util/valuePtrHelper.hpp"
#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT HandleType,
                                SQLHANDLE Handle,
                                SQLSMALLINT RecNumber,
                                _Out_writes_opt_(6) SQLCHAR* SqlStatePtr,
                                SQLINTEGER* NativeErrorPtr,
                                _Out_writes_opt_(BufferLength)
                                    SQLCHAR* MessageTextPtr,
                                SQLSMALLINT BufferLength,
                                _Out_opt_ SQLSMALLINT* TextLengthPtr) {
  /*
  Return a series of 1-indexed diagnostic records from various handles.
  If a record is requested beyond what is actually available, return
  SQL_NO_DATA instead.
  */
  WriteLog(LL_ERROR, "Entering SQLGetDiagRec");
  switch (HandleType) {
    case (SQL_HANDLE_ENV): {
      Environment* env = reinterpret_cast<Environment*>(Handle);
      WriteLog(LL_ERROR, "  Requesting diagnostics for environment handle");
      WriteLog(LL_ERROR,
               "  Requesting RecNumber: " + std::to_string(RecNumber));
      return SQL_NO_DATA;
    }
    case (SQL_HANDLE_DBC): {
      Connection* conn = reinterpret_cast<Connection*>(Handle);
      WriteLog(LL_ERROR, "  Requesting diagnostics for connection handle");
      WriteLog(LL_ERROR,
               "  Requesting RecNumber: " + std::to_string(RecNumber));
      ErrorInfo errorInfo = conn->getError();
      if (RecNumber == 1 and errorInfo.errorOccurred()) {
        writeNullTermStringToPtr<SQLINTEGER>(
            SqlStatePtr, errorInfo.sqlStateCode, nullptr);
        writeNullTermStringToPtr(
            MessageTextPtr, errorInfo.errorMessage, TextLengthPtr);
        *NativeErrorPtr = -1;
        return SQL_SUCCESS;
      } else {
        return SQL_NO_DATA;
      }
    }
    case (SQL_HANDLE_STMT): {
      Statement* statement = reinterpret_cast<Statement*>(Handle);
      WriteLog(LL_ERROR, "  Requesting diagnostics for statement handle");
      WriteLog(LL_ERROR,
               "  Requesting RecNumber: " + std::to_string(RecNumber));
      return SQL_NO_DATA;
    }
    case (SQL_HANDLE_DESC): {
      Descriptor* descriptor = reinterpret_cast<Descriptor*>(Handle);
      WriteLog(LL_ERROR, "  Requesting diagnostics for descriptor handle");
      WriteLog(LL_ERROR,
               "  Requesting RecNumber: " + std::to_string(RecNumber));
      return SQL_NO_DATA;
    }
    default: {
      WriteLog(LL_ERROR, "  ERROR: Unknown handle type in SQLGetDiagRec");
      return SQL_ERROR;
    }
  }
}
