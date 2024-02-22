#pragma once

#include <nlohmann/json.hpp>

extern "C" {
#include <database/query/query.h>
}

void json_to_database_query(const nlohmann::json& source, query& target);