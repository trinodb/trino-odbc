#include "../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <map>
#include <string>

#include "config/profileReader.hpp"
#include "handles/connHandle.hpp"

#include "../util/delimKvpHelper.hpp"
#include "../util/stringFromChar.hpp"
#include "../util/writeLog.hpp"

SQLRETURN SQL_API SQLDriverConnect(SQLHDBC ConnectionHandle,
                                   SQLHWND Windowhandle,
                                   _In_reads_(InConnectionChars)
                                       SQLCHAR* InConnectionChars,
                                   SQLSMALLINT StringLength1,
                                   _Out_writes_opt_(OutConnectionChars)
                                       SQLCHAR* OutConnectionChars,
                                   SQLSMALLINT BufferLength,
                                   _Out_opt_ SQLSMALLINT* StringLength2Ptr,
                                   SQLUSMALLINT DriverCompletion) {
  WriteLog(LL_TRACE, "Entering SQLDriverConnect");
  Connection* connection = reinterpret_cast<Connection*>(ConnectionHandle);

  if (InConnectionChars == nullptr) {
    WriteLog(LL_ERROR, "  ERROR: Connection string input is null");
    return SQL_ERROR;
  }

  WriteLog(LL_TRACE, "  Reading input connection string");
  std::string inputConnStr = stringFromChar(InConnectionChars, StringLength1);

  WriteLog(LL_TRACE, "  Input connection string was: " + inputConnStr);

  WriteLog(LL_TRACE, "  Parsing input connection string");
  std::map<std::string, std::string> kvps =
      parseKVPsFromSemicolonDelimStr(inputConnStr);

  if (kvps.count("dsn")) {
    WriteLog(LL_TRACE, "  An explicit DSN was provided to SQLDriverConnect");
    // This means there was an explict DSN passed in. We should use whatever
    // config we can from that DSN's profile
    std::string dsn            = kvps.at("dsn");
    DriverConfig defaultConfig = readDriverConfigFromProfile(dsn);
    std::map<std::string, std::string> defaultKvps =
        driverConfigToKVPs(defaultConfig);

    // Copy the default values into the parsed version of the
    // connection string, but only if the key isn't already present.
    // This will provide the defaults without overwriting any customization.
    for (auto it = defaultKvps.begin(); it != defaultKvps.end(); it++) {
      if (not kvps.count(it->first)) {
        kvps[it->first] = it->second;
      }
    }
    WriteLog(LL_TRACE, "The final connection config was as follows");
    WriteLog(LL_TRACE, kvps);
  } else {
    WriteLog(LL_TRACE,
             "  An explicit DSN was not provided to SQLDriverConnect");
  }

  WriteLog(LL_TRACE, "  Constructing driver config");
  DriverConfig config = driverConfigFromKVPs(kvps);

  // It's kind of unfortunate that we can't set the log level of the driver
  // until we've read the DSN in some way.
  WriteLog(LL_TRACE, "  Setting Log Level");
  setLogLevel(config.getLogLevelEnum());

  WriteLog(LL_TRACE, "  Configuring connection");
  try {
    connection->configure(config);

    WriteLog(LL_TRACE,
             "  Copying input connection string to output connection string");
    // Check if there's enough space in OutConnectionString
    if (OutConnectionChars && BufferLength > 0) {
      if (static_cast<int>(inputConnStr.size() + 1) <= BufferLength) {
        // Copy the entire string
        inputConnStr.copy(reinterpret_cast<char*>(OutConnectionChars),
                          inputConnStr.size());
        WriteLog(LL_TRACE, "  Connection string copied successfully.");
        // Ensure null-termination
        OutConnectionChars[inputConnStr.size()] = '\0';
      }
    } else {
      // OutConnectionString is NULL or BufferLength is 0, just return the
      // length
      WriteLog(LL_WARN, "  No output buffer provided.");
    }

    // If StringLength2Ptr is provided, set it to the length of the connection
    // string
    if (StringLength2Ptr) {
      *StringLength2Ptr = static_cast<SQLSMALLINT>(inputConnStr.size());
      WriteLog(LL_TRACE, "  Length of connection string set.");
    }

    connection->connected = true;

    WriteLog(LL_TRACE, "  Connection ready");
    return SQL_SUCCESS;
  } catch (std::exception& e) {
    WriteLog(LL_DEBUG,
             "  Error from SQLDriverConnect: " + std::string(e.what()));
    ErrorInfo error = ErrorInfo(e.what(), "HY000");
    connection->setError(error);
    return SQL_ERROR;
  }
}
