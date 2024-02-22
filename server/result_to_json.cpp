#include "result_to_json.hpp"

extern "C" {
#include <query/query_processor.h>
}

nlohmann::json element_to_json(const element& element) {
  nlohmann::json res;
  res["elementType"] = std::string(element.element_type->type_name);

  auto [attrs_it, success] = res.emplace("attributes", nlohmann::json::array());
  for (attribute_count_t i = 0; i < element.attribute_count; ++i) {
    switch (element.attributes[i].type) {
      case ATTR_TYPE_BOOL:
        attrs_it->emplace_back<nlohmann::json>(
            {{"attributeType", "bool"}, {"value", static_cast<int>(element.attributes[i].value.as_bool)}});
        break;
      case ATTR_TYPE_DOUBLE:
        attrs_it->emplace_back<nlohmann::json>(
            {{"attributeType", "double"}, {"value", element.attributes[i].value.as_double}});
        break;
      case ATTR_TYPE_INT32:
        attrs_it->emplace_back<nlohmann::json>(
            {{"attributeType", "int32"}, {"value", element.attributes[i].value.as_int32}});
        break;
      case ATTR_TYPE_INT64:
        attrs_it->emplace_back<nlohmann::json>(
            {{"attributeType", "int64"}, {"value", element.attributes[i].value.as_int64}});
        break;
      case ATTR_TYPE_STRING:
        attrs_it->emplace_back<nlohmann::json>(
            {{"attributeType", "string"}, {"value", std::string(element.attributes[i].value.as_string.data)}});
        break;
    }
  }

  return res;
}

nlohmann::json result_to_json(query_result* result) {
  nlohmann::json res;

  error_code err = query_result_reset(result);

  while (err == 0) {
    element* el;
    err = query_result_get_data(result, &el);
    if (err == 0) {
      res.emplace_back(element_to_json(*el));
      element_dtor(el);
      free(el);
    }

    err = query_result_next(result);
  }

  return res;
}