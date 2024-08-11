#include "columnDescription.hpp"

ColumnDescription::ColumnDescription(const json& columnInfo) {
  this->name          = columnInfo["name"];
  this->type          = columnInfo["type"];
  this->rawType       = columnInfo["typeSignature"]["rawType"];
  this->typeArguments = columnInfo["typeSignature"]["arguments"];
}

const std::string& ColumnDescription::getName() const {
  return this->name;
}

const std::string& ColumnDescription::getType() const {
  return this->type;
}

const std::string& ColumnDescription::getRawType() const {
  return this->rawType;
}

const json& ColumnDescription::getTypeArguments() const {
  return this->typeArguments;
}
