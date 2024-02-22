#include "json_to_response.hpp"

Response json_to_response(const nlohmann::json& src) {
  Response res = {};

  res.error = src["error"].get<int32_t>();
  if (res.error != 0) {
    return res;
  }

  for (const auto& element : src["result"]) {
    Element e;
    e.type = std::move(element["elementType"]);

    for (const auto& attr : element["attributes"]) {
      auto attr_type = attr["attributeType"];
      if (attr_type == "int64") {
        e.attrs.emplace_back(attribute{.type = ATTR_TYPE_INT64, .value = {.as_int64 = attr["value"].get<int64_t>()}});
      } else if (attr_type == "int32") {
        e.attrs.emplace_back(attribute{.type = ATTR_TYPE_INT32, .value = {.as_int32 = attr["value"].get<int32_t>()}});
      } else if (attr_type == "bool") {
        e.attrs.emplace_back(
            attribute{.type = ATTR_TYPE_BOOL, .value = {.as_bool = static_cast<bool>(attr["value"].get<int>())}});
      } else if (attr_type == "double") {
        e.attrs.emplace_back(attribute{.type = ATTR_TYPE_DOUBLE, .value = {.as_double = attr["value"].get<double>()}});
      } else if (attr_type == "string") {
        const auto& str = attr["value"].get<std::string>();
        char* target = (char*)calloc(1, str.size() + 1);

        e.attrs.emplace_back(attribute{.type = ATTR_TYPE_STRING, .value = {.as_string = {.data = target}}});

        std::memcpy(target, str.c_str(), str.size() + 1);
      }
    }

    res.elements.emplace_back(std::move(e));
  }

  return res;
}