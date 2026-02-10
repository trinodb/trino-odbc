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

void Connection::checkInputs(DriverConfig config) {
  if (config.getHostname().empty()) {
    throw std::invalid_argument("hostname cannot be empty");
  } else if (config.getPortNum() <= 0) {
    throw std::invalid_argument("port must be greater than zero");
  }
}

void Connection::configure(DriverConfig config) {
  // This instantiates a driver config object on the heap.
  // The destructor will clean it up if that's happened.
  checkInputs(config);

  this->connectionConfig = new ConnectionConfig(config.getHostname(),
                                                config.getPortNum(),
                                                config.getAuthMethodEnum(),
                                                config.getDSN(),
                                                config.getOidcDiscoveryUrl(),
                                                config.getClientId(),
                                                config.getClientSecret(),
                                                config.getOidcScope(),
                                                config.getGrantType(),
                                                config.getTokenEndpoint(),
                                                config.getAuthEndpoint(),
                                                config.getUsername(),
                                                config.getPassword(),
                                                config.getOidcEndpointMethodEnum());
}

void Connection::setError(ErrorInfo errorInfo) {
  this->errorInfo = errorInfo;
}

ErrorInfo Connection::getError() {
  return this->errorInfo;
}
