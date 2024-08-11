#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

json parseAccessToken(std::string& accessToken);
