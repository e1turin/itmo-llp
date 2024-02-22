#pragma once

#include <nlohmann/json.hpp>

extern "C" {
#include <database/api/element.h>
struct query_result;
}

nlohmann::json element_to_json(const element& element);
nlohmann::json result_to_json(query_result* result);