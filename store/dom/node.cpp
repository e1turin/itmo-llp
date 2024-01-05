
#include "node.h"

namespace dom {

Value::Type Value::getType() const {
  return Value::Type::kBoolean;
}

Int32Value::Int32Value(int32_t) {}
BoolValue::BoolValue(bool) {}
Float64Value::Float64Value(double) {}
StringValue::StringValue(std::string) {}

}
