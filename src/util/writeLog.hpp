#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>

// The numerical values are basically the "importance" of the log message.
// By setting a log level, you're setting the level at which you
// want to see the logging.
enum LogLevel {
  LL_TRACE = 0,
  LL_DEBUG = 1,
  LL_INFO  = 2,
  LL_WARN  = 3,
  LL_ERROR = 4,
  LL_NONE  = 5,
};

using json = nlohmann::json;

void WriteLog(LogLevel level, const char* message);

void WriteLog(LogLevel level, unsigned char* message, int length);

void WriteLog(LogLevel level, const json& jsonData);

void WriteLog(LogLevel level, const std::string& s);

void WriteLog(LogLevel level, const std::wstring& s);

void WriteLog(LogLevel level, const std::map<std::string, std::string>& m);

void WriteLog(LogLevel level, void* p);

void WriteLog(LogLevel level, unsigned int i);

void WriteLog(LogLevel level, int64_t i);

void WriteLog(LogLevel level, uint64_t i);

void setLogLevel(LogLevel level);

LogLevel getLogLevel();
