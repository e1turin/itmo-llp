#include "validate.hpp"

bool validate(const nlohmann::json& schema_doc, const nlohmann::json& target_doc) {
  valijson::Schema schema;
  valijson::SchemaParser parser;
  valijson::adapters::NlohmannJsonAdapter schema_adapter(schema_doc);
  parser.populateSchema(schema_adapter, schema);

  valijson::Validator validator;
  valijson::adapters::NlohmannJsonAdapter target_adapter(target_doc);

  if (!validator.validate(schema, target_adapter, nullptr)) {
    return false;
  }
  return true;
}
