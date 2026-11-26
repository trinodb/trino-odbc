#include "basicAuthProvider.hpp"

#include <map>
#include <stdexcept>
#include <string>

#include "../../util/writeLog.hpp"

std::string toBase64(const std::string& input) {
  static const char* alphabet =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((input.size() + 2) / 3) * 4);

  size_t i = 0;
  while (i + 2 < input.size()) {
    unsigned int n = (static_cast<unsigned char>(input[i]) << 16) |
                     (static_cast<unsigned char>(input[i + 1]) << 8) |
                     static_cast<unsigned char>(input[i + 2]);
    out.push_back(alphabet[(n >> 18) & 0x3F]);
    out.push_back(alphabet[(n >> 12) & 0x3F]);
    out.push_back(alphabet[(n >> 6) & 0x3F]);
    out.push_back(alphabet[n & 0x3F]);
    i += 3;
  }

  size_t remaining = input.size() - i;
  if (remaining == 1) {
    unsigned int n = static_cast<unsigned char>(input[i]) << 16;
    out.push_back(alphabet[(n >> 18) & 0x3F]);
    out.push_back(alphabet[(n >> 12) & 0x3F]);
    out.append("==");
  } else if (remaining == 2) {
    unsigned int n = (static_cast<unsigned char>(input[i]) << 16) |
                     (static_cast<unsigned char>(input[i + 1]) << 8);
    out.push_back(alphabet[(n >> 18) & 0x3F]);
    out.push_back(alphabet[(n >> 12) & 0x3F]);
    out.push_back(alphabet[(n >> 6) & 0x3F]);
    out.push_back('=');
  }

  return out;
}

class BasicAuthConfig : public AuthConfig {
  public:
    BasicAuthConfig(std::string hostname,
                    unsigned short port,
                    std::string connectionName,
                    std::string user,
                    std::string password)
        : AuthConfig(hostname, port, connectionName) {
      if (user.empty()) {
        throw std::invalid_argument(
            "Basic Auth requires a user name (UID=... in the connection "
            "string)");
      }
      if (hostname.rfind("https://", 0) != 0) {
        WriteLog(LL_WARN,
                 "  WARNING: Basic Auth is configured but the hostname does "
                 "not start with https://. Trino rejects passwords over "
                 "plain HTTP.");
      }
      this->headers.insert({"X-Trino-User", user});
      this->headers.insert(
          {"Authorization", "Basic " + toBase64(user + ":" + password)});
      WriteLog(LL_DEBUG, "  Basic Auth configured for user " + user);
    }

    bool const isExpired() override {
      // Credentials are sent on every request; nothing ever expires.
      return false;
    }
    void
    refresh(CURL* curl,
            std::string* responseData,
            std::map<std::string, std::string>* responseHeaderData) override {
      // Nothing to refresh.
    }
    ~BasicAuthConfig() override = default;
};

std::unique_ptr<AuthConfig> getBasicAuthConfigPtr(std::string hostname,
                                                  unsigned short port,
                                                  std::string connectionName,
                                                  std::string user,
                                                  std::string password) {
  return std::make_unique<BasicAuthConfig>(
      hostname, port, connectionName, user, password);
}
