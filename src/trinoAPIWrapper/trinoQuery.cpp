#include <algorithm>
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <ranges>
#include <thread>

#include "trinoExceptions.hpp"
#include "trinoQuery.hpp"
#include <stdexcept>

#include "../util/delimKvpHelper.hpp"
#include "../util/stringTrim.hpp"
#include "../util/writeLog.hpp"

// How long should we poll between requests to Trino's nextUri?
int API_POLL_INTERVAL_MS = 25;

UpdateStatus TrinoQuery::updateSelfFromResponse() {
  WriteLog(LL_TRACE, "  Entering TrinoQuery::updateSelfFromResponse");
  json response_json = json::parse(this->connectionConfig->responseData);
  WriteLog(LL_DEBUG, "  Response is Parsed");
  UpdateStatus updateStatus;

  if (response_json.contains("error")) {
    this->error = true;
  }

  if (response_json.contains("queryId")) {
    this->queryId = response_json["queryId"];
  }

  if (response_json.contains("infoUri")) {
    this->infoUri = response_json["infoUri"];
  } else {
    this->infoUri.clear();
  }

  if (response_json.contains("partialCancelUri")) {
    this->partialCancelUri = response_json["partialCancelUri"];
  } else {
    this->partialCancelUri.clear();
  }

  if (response_json.contains("nextUri")) {
    this->nextUri = response_json["nextUri"];
  } else {
    // This marks the point after which no more data will arrive,
    // so the query is now completed.
    this->completed = true;
    this->nextUri.clear();
  }

  if (response_json.contains("columns") and this->columnDescriptions.empty()) {
    WriteLog(LL_TRACE, "  Parsing column info from TrinoQuery data result");
    this->columnsJson = response_json["columns"];
    std::vector<ColumnDescription> columnDescriptions;
    std::transform(this->columnsJson.begin(),
                   this->columnsJson.end(),
                   std::back_inserter(columnDescriptions),
                   [](const json& json) { return ColumnDescription(json); });
    this->columnDescriptions   = columnDescriptions;
    updateStatus.gotColumnInfo = true;
    for (std::function f : this->onColumnDataCallbacks) {
      f(this);
    }
  }

  if (response_json.contains("data")) {
    WriteLog(LL_TRACE, "  Adding data to TrinoQuery data result");
    updateStatus.gotRowData = true;
    size_t existingRows     = this->dataJson.size();
    size_t newRows          = response_json["data"].size();
    // Optimization: pre-allocate enough room to hold all the
    // rows of data up front, this avoids resizing over and
    // over to hold an unknown quantity of rows.
    this->dataJson.reserve(existingRows + newRows);
    this->dataJson.insert(this->dataJson.end(),
                          response_json["data"].begin(),
                          response_json["data"].end());
  }

  // All "real" queries contain a state, but sideloaded
  // queries from ODBC functions might not, so we need
  // to handle a no-state response gracefully.
  if (response_json.contains("stats")) {
    if (response_json["stats"].contains("state")) {
      this->status = response_json["stats"]["state"];
    }
  }

  WriteLog(LL_TRACE, "  Exiting TrinoQuery::updateSelfFromResponse");
  return updateStatus;
}

void TrinoQuery::onConnectionReset(ConnectionConfig* connectionConfig) {
  // If the connection is about to be reset, terminate any in-flight
  // queries first so they aren't left abandoned.
  this->terminate();
}

TrinoQuery::TrinoQuery(ConnectionConfig* connectionConfig) {
  this->connectionConfig = connectionConfig;
  this->connectionConfig->registerDisconnectCallback(
      std::bind(&TrinoQuery::onConnectionReset, this, std::placeholders::_1));
}

TrinoQuery::~TrinoQuery() {
  this->connectionConfig->unregisterDisconnectCallback(
      std::bind(&TrinoQuery::onConnectionReset, this, std::placeholders::_1));
}

void TrinoQuery::setQuery(std::string query) {
  this->query = query;
}

const std::string& TrinoQuery::getQuery() const {
  return this->query;
}

void TrinoQuery::post() {
  CURL* curl = this->connectionConfig->getCurl();

  std::string statementURL = this->connectionConfig->getStatementUrl();
  curl_easy_setopt(curl, CURLOPT_URL, statementURL.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());

  CURLcode res = curl_easy_perform(curl);

  long httpStatusCode = this->connectionConfig->getLastHTTPStatusCode();

  if (httpStatusCode == 200 and res == CURLE_OK) {
    updateSelfFromResponse();
    if (this->nextUri.empty()) {
      WriteLog(LL_ERROR,
               "  Error POSTing query. No next_uri in response " +
                   std::to_string(httpStatusCode));
      throw std::runtime_error("No NextURI in Trino POST response");
    }
  } else {
    // If we get here, there was a problem posting the query.
    WriteLog(LL_ERROR,
             "  Error POSTing query. CURL status code was " +
                 std::to_string(httpStatusCode));
    throw std::runtime_error("Unexpected Trino POST status: " +
                             std::to_string(httpStatusCode));
  }
}

void TrinoQuery::poll(TrinoQueryPollMode mode) {
  if (this->completed) {
    return;
  }

  CURL* curl    = this->connectionConfig->getCurl();
  int pollCount = 1;
  while (!this->completed) {
    // Since we're reusing the curl handle, we need to clear any
    // data returned from it. This is kind of ugly, but it is
    // highly efficient.
    this->connectionConfig->responseData.clear();
    this->connectionConfig->responseHeaderData.clear();
    curl_easy_setopt(curl, CURLOPT_URL, this->nextUri.c_str());

    CURLcode res;
    res = curl_easy_perform(curl);
    UpdateStatus updateStatus;
    if (res == CURLE_OK) {
      updateStatus = updateSelfFromResponse();
    }

    if (mode == JustOnce) {
      break;
    }
    if (mode == UntilColumnsLoaded && not this->columnsJson.empty()) {
      break;
    }
    if (mode == UntilNewData and not this->completed) {
      // If it's not complete, only stop polling if we got new row data.
      if (updateStatus.gotRowData) {
        break;
      }
    }
    //  If we learned something from the last request, reset the poll counter
    //  so we can try to read more data. If we didn't learn anything from
    //  the last request, we should sleep for a bit to give the server some
    //  time to get ready for the next request.
    if (updateStatus.gotRowData or updateStatus.gotColumnInfo) {
      pollCount = 0;
    } else {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(pollCount * API_POLL_INTERVAL_MS));
    }
    pollCount++;
  }
}

/*
 Canceling a query causes it to gracefully stop.
 It may return a few more rows before finishing up,
 but it will attempt to stop before completion.
 This is accomplished by sending a DELETE to the
 partialCancelUri.

 NOTE: Testing showed this wasn't actually cancelling
 queries for me. TrinoQuery::terminate() worked better.
*/
void TrinoQuery::cancel() {
  if (this->partialCancelUri.size() > 0) {
    CURL* curl = this->connectionConfig->getCurl();
    curl_easy_setopt(curl, CURLOPT_URL, this->partialCancelUri.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);
    UpdateStatus updateStatus;
    if (res == CURLE_OK) {
      // There's nothing to parse from the result of the DELETE
      // we sent to Trino, so the CURLE_OK means it was successful.
      // However, we actually need to finish reading the data
      // in order for Trino to mark the query as completed
      // after canceling the query. Otherwise it remains stuck
      // in the "FINISHING" state.
      WriteLog(LL_WARN, "Query Cancellation Sent. Polling to completion");
      this->poll(ToCompletion);
    }
  }
}

/*
 Terminating a query causes it to stop running ASAP.
 This is accomplished by sending a DELETE to the nextUri.
*/
void TrinoQuery::terminate() {
  if (not this->getIsCompleted() and this->nextUri.size() > 0) {
    CURL* curl = this->connectionConfig->getCurl();
    curl_easy_setopt(curl, CURLOPT_URL, this->nextUri.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);
    UpdateStatus updateStatus;
    if (res == CURLE_OK) {
      // A success status on the terminate command means it
      // was successful. There's nothing to read after.
      // We can reset the statement in case someone tries
      // to reuse it.
      WriteLog(LL_WARN, "Query Termination Sent. Resetting query object");
      this->reset();
      return;
    } else {
      throw std::runtime_error("Trino query termination failed");
    }
  }
}

const int64_t TrinoQuery::getAbsoluteRowCount() const {
  // The ODBC convention for row counts is that -1 represents
  // an as-yet unknown number of rows.
  if (this->completed) {
    return this->getCurrentRowCount();
  } else {
    return -1;
  }
}

const int64_t TrinoQuery::getCurrentRowCount() const {
  // It can be useful to know how many rows are currently available.
  // However, the offset position needs to be included in this value
  // to provide the facade that the checkpointed rows that have
  // been discarded from memory are still around. Add one to the
  // offset position to turn it into a length/size.
  return (this->rowOffsetPosition + 1) + this->dataJson.size();
}

const int16_t TrinoQuery::getColumnCount() {
  if (this->columnDescriptions.empty()) {
    this->poll(UntilColumnsLoaded);
  }
  // Yes, precision is lost here. However, the maximum column
  // count in a SQL query can be limited to a 16 bit integer
  // without any risk to normal everyday tables.
  return static_cast<int16_t>(this->columnsJson.size());
}

const std::vector<ColumnDescription>& TrinoQuery::getColumnDescriptions() {
  if (this->columnDescriptions.empty()) {
    this->poll(UntilColumnsLoaded);
  }
  return this->columnDescriptions;
}

const bool TrinoQuery::getIsCompleted() const {
  return this->completed;
}

void TrinoQuery::sideloadResponse(json artificialResponse) {
  /*
   Most ODBC functions return a status code, not an actual result.
   Also, ODBC is, at its heart, a protocol for reading results.
   When an application or the driver manager needs to obtain
   a result of any kind, it leverages the mechanisms to read data
   as if it was a query, regardless of whether or not the data
   actually comes from a database.
   The sideload method makes it easy to drop a response in
   that doesn't actually come from the database, such as
   the type information for supported types for the driver.
   */
  this->connectionConfig->responseData = artificialResponse.dump();
  this->updateSelfFromResponse();
}

/*
  Reset happens when an application wants to reuse a cursor with
  a different query. We need to clear out the results such that
  this is ready to be reused. The connection information
  can remain in place, as can the registered column change callback,
  but all the query data and metadata needs to be reset.
*/
void TrinoQuery::reset() {
  WriteLog(LL_TRACE, "  TrinoQuery is resetting");
  this->query.clear();
  this->queryId.clear();
  this->infoUri.clear();
  this->partialCancelUri.clear();
  this->nextUri.clear();
  this->status.clear();
  this->columnsJson.clear();
  this->dataJson.clear();
  this->columnDescriptions.clear();
  this->error             = false;
  this->completed         = false;
  this->rowOffsetPosition = -1;
}

void TrinoQuery::registerColumnDataChangeCallback(
    std::function<void(TrinoQuery*)> f) {
  this->onColumnDataCallbacks.push_back(f);
}

const bool TrinoQuery::hasColumnData() const {
  return not this->columnDescriptions.empty();
}

/*
  We don't want dataJson to grow without bounds, otherwise
  we will run out of system memory on queries with lots of data.

  The solution is to allow callers to checkpoint their current position.
  This signals to the query that all rows have been read up to
  and including the completedIndex and that any memory consumed
  by those earlier rows can be freed.

  You wouldn't want to call this too frequently, since it resizes
  a vector and would be quite computationally expensive.
*/
void TrinoQuery::checkpointRowPosition(int64_t completedIndex) {
  // Don't do anything if we try to checkpoint before any
  // rows are read (index -1).
  if (completedIndex < 0) {
    return;
  }
  // Get the position in dataJson that represents what has been
  // completed already.
  int64_t dataJsonPosition = completedIndex;
  if (this->rowOffsetPosition > -1) {
    dataJsonPosition -= (this->rowOffsetPosition + 1);
  }

  // Watch out for unsigned integer underflow in this comparison!
  if (dataJsonPosition == static_cast<int64_t>(dataJson.size()) - 1) {
    // If we're erasing the whole thing, just clear it.
    dataJson.clear();
  } else {
    // Otherwise, erase up to dataJsonPosition, being careful of the
    // type of the offset for x86 vs x64 compilation.
    dataJson.erase(dataJson.begin(),
                   dataJson.begin() +
                       static_cast<std::vector<int64_t>::difference_type>(
                           dataJsonPosition));
  }
  this->rowOffsetPosition = completedIndex;
}

/*
We need to hide the indexing into the dataJson vector
so that we can implement the rowOffsetPosition offset. This
gives callers the ability to track row offsets well beyond the
number of rows that actually fit into memory from a query.
*/
const json& TrinoQuery::getRowAtIndex(int64_t index) const {
  if (this->rowOffsetPosition > -1) {
    return this->dataJson[static_cast<std::vector<json>::size_type>(
        index - (this->rowOffsetPosition + 1))];
  } else {
    return this->dataJson[static_cast<std::vector<json>::size_type>(index)];
  }
}
