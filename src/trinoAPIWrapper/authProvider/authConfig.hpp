#pragma once

#include <curl/curl.h>
#include <map>
#include <string>

class AuthConfig {
  public:
    // Any headers that must be included on all requests go here.
    std::map<std::string, std::string> headers = {
        {"X-Trino-Source", "TrinoODBCDriver"},
    };
    std::string hostname;
    unsigned short port;
    std::string connectionName;

    AuthConfig(std::string hostname,
               unsigned short port,
               std::string connectionName) {
      this->hostname       = hostname;
      this->port           = port;
      this->connectionName = connectionName;
    }

    // The '=0' on the end makes these "pure virtual" methods,
    // transforming this into an abstract bass class.
    virtual bool const isExpired()                                       = 0;
    virtual void refresh(CURL* curl,
                         std::string* responseData,
                         std::map<std::string, std::string>* headerData) = 0;

    // Virtual destructors are considered "best practice" for virtual classes.
    virtual ~AuthConfig() = default;
};
