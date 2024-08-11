#pragma once
#include "../../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include "../../trinoAPIWrapper/environmentConfig.hpp"

class Environment {
  public:
    Environment();
    ~Environment();
    SQLINTEGER AttrODBCVersion       = SQL_OV_ODBC3;
    SQLINTEGER AttrConnectionPooling = SQL_CP_DEFAULT;
    EnvironmentConfig* environmentConfig;
};
