#pragma once

#include <memory>
#include <string>

#include "authConfig.hpp"

std::unique_ptr<AuthConfig>
getClientCredAuthProvider(std::string hostname,
                          unsigned short port,
                          std::string connectionName,
                          std::string oidcDiscoveryUrl,
                          std::string clientId,
                          std::string clientSecret,
                          std::string oidcScope);
