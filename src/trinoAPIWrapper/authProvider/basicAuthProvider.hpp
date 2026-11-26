#pragma once

#include <memory>
#include <string>

#include "authConfig.hpp"

// HTTP Basic authentication (user name + password), as used by Trino's
// PASSWORD authenticator (LDAP, password file, etc.). Trino only accepts
// this over HTTPS, so the hostname should start with "https://".
std::unique_ptr<AuthConfig> getBasicAuthConfigPtr(std::string hostname,
                                                  unsigned short port,
                                                  std::string connectionName,
                                                  std::string user,
                                                  std::string password);

// Standard (RFC 4648) base64 encoding with padding. Exposed for testing.
std::string toBase64(const std::string& input);
