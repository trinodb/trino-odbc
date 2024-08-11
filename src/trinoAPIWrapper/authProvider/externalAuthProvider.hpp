#pragma once

#include <memory>
#include <string>

#include "authConfig.hpp"

std::unique_ptr<AuthConfig> getExternalAuthConfigPtr(
    std::string hostname, unsigned short port, std::string connectionName);
