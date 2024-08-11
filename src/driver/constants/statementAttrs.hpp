#pragma once

/*
 Driver-defined statement attribute to get
 the raw statement handle without the ODBC
 driver manager proxy layer. Useful mostly
 for unit testing.
*/
#define SQL_ATTR_RAW_STATEMENT_HANDLE 1001

/*
 Driver-defined statement attribute to set
 the default polling method used by SQLFetch.
 Values are

 * JustOnce (0)
 * UntilNewData (1)
 * UntilColumnsLoaded (2)
 * ToCompletion (3)

 The default is UntilNewData, but ToCompletion
 is useful for unit tests that want to evaluate
 fetch performance independent of poll performance.
*/
#define SQL_ATTR_DEFAULT_FETCH_POLL_MODE 1002
