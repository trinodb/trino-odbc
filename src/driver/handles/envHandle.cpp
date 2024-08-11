#include "envHandle.hpp"

Environment::Environment() {
  this->environmentConfig = new EnvironmentConfig();
}

Environment::~Environment() {
  delete this->environmentConfig;
}
