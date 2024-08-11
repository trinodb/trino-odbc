#include "noAuthProvider.hpp"

#include <map>
#include <string>


// The default AuthConfig struct is sufficient to describe what
// needs to happen for a trino coordinator with auth disabled.
class NoAuthAuthConfig : public AuthConfig {
  public:
    NoAuthAuthConfig(std::string hostname,
                     unsigned short port,
                     std::string connectionName)
        : AuthConfig(hostname, port, connectionName) {
      // TODO: find a way to let the DSN config set an explicit
      //       username for this instead of "TestUser".
      this->headers.insert({"X-Trino-User", "TestUser"});
    }

    bool const isExpired() override {
      return false;
    }
    void
    refresh(CURL* curl,
            std::string* responseData,
            std::map<std::string, std::string>* responseHeaderData) override {
      // There's nothing to refresh.
    }
    ~NoAuthAuthConfig() override = default;
};

std::unique_ptr<AuthConfig> getNoAuthConfigPtr(std::string hostname,
                                               unsigned short port,
                                               std::string connectionName) {
  return std::make_unique<NoAuthAuthConfig>(hostname, port, connectionName);
}
