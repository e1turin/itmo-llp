#pragma once

#include "response.hpp"

#include <vector>

#include <nlohmann/json.hpp>

Response json_to_response(const nlohmann::json& src);