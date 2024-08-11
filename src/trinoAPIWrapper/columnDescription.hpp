#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class ColumnDescription {
  private:
    std::string name;
    std::string type;
    std::string rawType;
    json typeArguments;

  public:
    ColumnDescription(const json& columnInfo);
    const std::string& getName() const;
    const std::string& getType() const;
    const std::string& getRawType() const;
    const json& getTypeArguments() const;
};
