
#include "value.h"

#include <cstring>
#include <stdexcept>

namespace dom {

void Value::init_tagged(Tag t) {
  memset(payload_, 0, sizeof payload_);
  payload_[0] = static_cast<std::uint8_t>(t);
}

NullValue::NullValue() {
  init_tagged(Tag::kNull);
}

BoolValue::BoolValue(bool b) {
  init_tagged(Tag::kBoolean);
  *(cast_data<bool>()) = b;
}
bool BoolValue::get_bool() const {
  return *cast_data<bool>();
}

Int32Value::Int32Value(int32_t i) {
  init_tagged(Tag::kInt32);
  *(cast_data<int32_t>()) = i;
}
std::int32_t Int32Value::get_int() const {
  return *cast_data<std::int32_t>();
}

Float32Value::Float32Value(float f) {
  init_tagged(Tag::kFloat32);
  *(cast_data<float>()) = f;
}
float Float32Value::get_float() const {
  return *cast_data<float>();
}

StringValue::StringValue(mem::Offset offset) {
  init_tagged(Tag::kString);
  //TODO: change to full size_t usage instead limited uint32_t
  if (offset.value() > UINT32_MAX) {
    throw std::runtime_error{"string data offset overflow"};
  }
  *(cast_data<std::uint32_t>()) = offset.value();
}

ObjectValue::ObjectValue(mem::Offset offset) {
  init_tagged(Tag::kObject);
  //TODO: change to full size_t usage instead limited uint32_t
  if (offset.value() > UINT32_MAX) {
    throw std::runtime_error{"object data offset overflow"};
  }
  *(cast_data<std::uint32_t>()) = offset.value();
}
ObjectValue::ObjectValue() : ObjectValue{mem::Offset{0}} {}

} // namespace dom
