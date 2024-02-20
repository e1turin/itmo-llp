
#include "value.h"

#include <cstring>

namespace dom {

void Value::init_tagged(Tag t) {
  tag_ = t;
  memset(payload_, 0, sizeof payload_);
}

void Value::init_tagged_pointer(const Tag t, void *p) {
  //TODO inline tag
  init_tagged(t);
  *(cast_data<uintptr_t>()) = reinterpret_cast<uintptr_t>(p);
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

} // namespace dom
