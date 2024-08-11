#pragma once
#include "../../util/windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <functional>

#include "descriptorHandle.hpp"
#include "handleErrorInfo.hpp"

#include "../../trinoAPIWrapper/connectionConfig.hpp"
#include "../../trinoAPIWrapper/trinoQuery.hpp"

class Statement {
  private:
    void columnsChangedCallback(TrinoQuery* trinoQuery);
    // SQLFetch needs us to keep track of our position in iterating
    // the results so subsequent calls to fetch can return data
    // starting at the right place. We start at position 0, which
    // indicates that no rows are ready to process.
    SQLLEN fetchedPosition = -1;
    ErrorInfo errorInfo;

  public:
    Statement(ConnectionConfig* connectionConfig);
    ~Statement();
    // Has this statement been executed?
    bool executed = false;
    // Did we confirm execution of this statement to SQLFetch?
    bool fetchExecuteConfirmed = false;
    // The underlying trino query utility class.
    TrinoQuery* trinoQuery;
    // The method used in SQLFetch for polling trino.
    TrinoQueryPollMode fetchPollMode = UntilNewData;

    // The ODBC protocol assumes these descriptors are
    // instantiated on all statements.
    Descriptor* appRowDesc;
    Descriptor* impRowDesc;
    Descriptor* appParamDesc;
    Descriptor* impParamDesc;

    void reset();
    void terminate();
    Descriptor* getRowDescriptor();
    Descriptor* getParamDescriptor();
    SQLLEN getFetchedPosition();
    void setFetchedPosition(SQLLEN pos);

    void setError(ErrorInfo errorInfo);
    ErrorInfo getError();
};
