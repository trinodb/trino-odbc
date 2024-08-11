#include "connHandle.hpp"

Connection::Connection(EnvironmentConfig* environmentConfig) {
  this->environmentConfig = environmentConfig;
}

Connection::~Connection() {
  if (this->connectionConfig) {
    delete this->connectionConfig;
  }
}

void Connection::disconnect() {
  this->connectionConfig->disconnect();
}

std::string Connection::getServerVersion() {
  return this->connectionConfig->getTrinoServerVersion();
}

void Connection::configure(DriverConfig config) {
  // This instantiates a driver config object on the heap.
  // The destructor will clean it up if that's happened.
  this->connectionConfig = new ConnectionConfig(config.getHostname(),
                                                config.getPortNum(),
                                                config.getAuthMethodEnum(),
                                                config.getDSN(),
                                                config.getOidcDiscoveryUrl(),
                                                config.getClientId(),
                                                config.getClientSecret(),
                                                config.getOidcScope());
}

void Connection::setError(ErrorInfo errorInfo) {
  this->errorInfo = errorInfo;
}

ErrorInfo Connection::getError() {
  return this->errorInfo;
}
