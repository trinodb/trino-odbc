#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "columnDescription.hpp"
#include "connectionConfig.hpp"

using json = nlohmann::json;

struct UpdateStatus {
    bool gotColumnInfo = false;
    bool gotRowData    = false;
};

enum TrinoQueryPollMode {
  JustOnce,
  UntilNewData,
  UntilColumnsLoaded,
  ToCompletion,
};

// Need to allow a few tests access to private variables
// in this class.
// * MemoryReclamationTest needs to view the size of private
//   collections to ensure memory is being reclaimed.
// * FetchGetDataPerformanceTest and FetchBindPerformanceTest
//   need to call poll(ToCompletion)
// For all of these, we do a forward declaration here
// and make this a friend class.
class MemoryReclamationTest;
class FetchGetDataPerformanceTest;
class FetchBindPerformanceTest;

class TrinoQuery {
  private:
    ConnectionConfig* connectionConfig;
    std::string query = "UNSET";
    std::string queryId;
    std::string infoUri;
    std::string partialCancelUri;
    std::string nextUri;
    std::string status;
    std::vector<json> columnsJson;
    std::vector<json> dataJson;
    std::vector<ColumnDescription> columnDescriptions;
    bool error     = false;
    bool completed = false;
    std::vector<std::function<void(TrinoQuery*)>> onColumnDataCallbacks;
    int64_t rowOffsetPosition = -1;
    UpdateStatus updateSelfFromResponse();
    void onConnectionReset(ConnectionConfig* connectionConfig);

    friend class MemoryReclamationTest;

  public:
    TrinoQuery(ConnectionConfig* connectionConfig);
    ~TrinoQuery();
    void setQuery(std::string query);
    const std::string& getQuery() const;
    void post();
    void cancel();
    void terminate();
    void poll(TrinoQueryPollMode mode);
    const int64_t getCurrentRowCount() const;
    const int64_t getAbsoluteRowCount() const;
    const int16_t getColumnCount();
    const std::vector<ColumnDescription>& getColumnDescriptions();
    const bool getIsCompleted() const;
    void sideloadResponse(json artificialResponse);
    void reset();
    void registerColumnDataChangeCallback(std::function<void(TrinoQuery*)> f);
    const bool hasColumnData() const;
    void checkpointRowPosition(int64_t completedIndex);
    const json& getRowAtIndex(int64_t) const;
};
