#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>

#include <algorithm>
#include <string>
#include <vector>

#include "../util/writeLog.hpp"

std::vector<SQLUSMALLINT> SUPPORTED_FUNCTIONS{
    // ODBC Core Support
    SQL_API_SQLALLOCHANDLE,
    SQL_API_SQLBINDCOL,
    SQL_API_SQLBINDPARAMETER,
    SQL_API_SQLCANCEL,
    SQL_API_SQLCANCELHANDLE,
    SQL_API_SQLCLOSECURSOR,
    SQL_API_SQLCOLATTRIBUTE,
    SQL_API_SQLCOLUMNS,
    SQL_API_SQLCONNECT,
    SQL_API_SQLCOPYDESC,
    SQL_API_SQLDATASOURCES,
    SQL_API_SQLDESCRIBECOL,
    SQL_API_SQLDISCONNECT,
    SQL_API_SQLDRIVERCONNECT,
    SQL_API_SQLDRIVERS,
    SQL_API_SQLENDTRAN,
    SQL_API_SQLEXECDIRECT,
    SQL_API_SQLEXECUTE,
    SQL_API_SQLEXTENDEDFETCH,
    SQL_API_SQLFETCH,
    SQL_API_SQLFETCHSCROLL,
    SQL_API_SQLFREEHANDLE,
    SQL_API_SQLFREESTMT,
    SQL_API_SQLGETCONNECTATTR,
    SQL_API_SQLGETCURSORNAME,
    SQL_API_SQLGETDATA,
    SQL_API_SQLGETDESCFIELD,
    SQL_API_SQLGETDESCREC,
    SQL_API_SQLGETDIAGREC,
    SQL_API_SQLGETENVATTR,
    SQL_API_SQLGETFUNCTIONS,
    SQL_API_SQLGETINFO,
    SQL_API_SQLGETSTMTATTR,
    SQL_API_SQLGETTYPEINFO,
    SQL_API_SQLNATIVESQL,
    SQL_API_SQLNUMPARAMS,
    SQL_API_SQLNUMRESULTCOLS,
    SQL_API_SQLPARAMDATA,
    SQL_API_SQLPREPARE,
    SQL_API_SQLPUTDATA,
    SQL_API_SQLROWCOUNT,
    SQL_API_SQLSETCONNECTATTR,
    SQL_API_SQLSETCURSORNAME,
    SQL_API_SQLSETDESCFIELD,
    SQL_API_SQLSETDESCREC,
    SQL_API_SQLSETENVATTR,
    SQL_API_SQLSETSCROLLOPTIONS,
    // Deprecated ODBC2.0 Functions
    SQL_API_SQLSETCONNECTOPTION,
    SQL_API_SQLALLOCENV,
    SQL_API_SQLALLOCCONNECT,
    SQL_API_SQLALLOCSTMT,
    SQL_API_SQLFREEENV,
    SQL_API_SQLFREECONNECT,
    SQL_API_SQLFREESTMT,
    SQL_API_SQLSETSTMTATTR,
    SQL_API_SQLSTATISTICS,
    SQL_API_SQLTABLES,
    // ODBC Level 1 Support
    // SQL_API_SQLBROWSECONNECT,
    // SQL_API_SQLBULKOPERATIONS,
    SQL_API_SQLMORERESULTS,
    // SQL_API_SQLPRIMARYKEYS,
    // SQL_API_SQLPROCEDURECOLUMNS,
    // SQL_API_SQLPROCEDURES,
    // SQL_API_SQLSETPOS,
    // ODBC Level 2 Support
    // SQL_API_SQLCOLUMNPRIVILEGES,
    // SQL_API_SQLDESCRIBEPARAM,
    // SQL_API_SQLFOREIGNKEYS,
    // SQL_API_SQLTABLEPRIVILEGES,
};

const SQLUSMALLINT bitsInASmallInt = sizeof(SQLSMALLINT) * 8;

void setSupportedBit(SQLUSMALLINT* Supported, SQLSMALLINT functionId) {
  SQLUSMALLINT index       = functionId / bitsInASmallInt;
  SQLUSMALLINT bitPosition = functionId % bitsInASmallInt;
  Supported[index] |= (1 << bitPosition);
}

SQLRETURN SQL_API SQLGetFunctions(
    SQLHDBC hdbc,
    SQLUSMALLINT FunctionId,
    _Out_writes_opt_(_Inexpressible_(
        "Buffer length pfExists points to depends on fFunction value."))
        SQLUSMALLINT* Supported) {
  WriteLog(LL_TRACE, "Entering SQLGetFunctions");
  if (Supported == nullptr) {
    WriteLog(LL_ERROR, "  ERROR: Supported array is null");
    return SQL_ERROR;
  }

  WriteLog(LL_TRACE, "  Function ID is: " + std::to_string(FunctionId));

  // Per the documentation, requests for ODBC3 all functions
  // will provide an array of length 250 and functions as a
  // bitfield of length 4000.
  if (FunctionId == SQL_API_ODBC3_ALL_FUNCTIONS) {
    // Note: By
    // 1. Including <limits> and
    // 2. Setting the fill value to
    //    std::numeric_limits<SQLUSMALLINT>::max() instead of 0
    // you can trick the odbc driver manager into thinking that every
    // ODBC function is implemented in the driver. This is useful
    // to see what _would_ have been called if it was available, and may
    // give direction on what functions you should consider adding
    // to the driver to improve compatibiility with your client app.
    std::fill_n(Supported, SQL_API_ODBC3_ALL_FUNCTIONS_SIZE, 0);

    for (auto i : SUPPORTED_FUNCTIONS) {
      WriteLog(LL_TRACE, "  Setting function id: " + std::to_string(i));
      setSupportedBit(Supported, i);
    }
  }

  // Per the documentation, requests for all functions (not ODBC3)
  // provisions an array of length 100, where every 16 bits function
  // as a single true/false value
  else if (FunctionId == SQL_API_ALL_FUNCTIONS) {
    std::fill_n(Supported, 100, SQL_FALSE);
    for (auto i : SUPPORTED_FUNCTIONS) {
      if (i < 100) {
        Supported[i] = SQL_TRUE;
      }
    }
  }

  // All other requests are for a specific function ID
  else if (std::find(SUPPORTED_FUNCTIONS.begin(),
                     SUPPORTED_FUNCTIONS.end(),
                     FunctionId) != SUPPORTED_FUNCTIONS.end()) {
    *Supported = SQL_TRUE;
  } else {
    *Supported = SQL_FALSE;
  }

  return SQL_SUCCESS;
}
