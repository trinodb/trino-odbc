#include <curl/curl.h>
#include <iostream>

#include "environmentConfig.hpp"

EnvironmentConfig::EnvironmentConfig() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

EnvironmentConfig::~EnvironmentConfig() {
  curl_global_cleanup();
}
