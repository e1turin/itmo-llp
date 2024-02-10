
#include "node.h"

#include <cstring>

namespace dom {

void Value::init_tagged(Tag t) {
  tag_ = t;
  memset(payload_, 0, sizeof payload_);
}

void Value::init_tagged_pointer(const Tag t, void *p) {
  init_tagged(t);
  *(cast_data<uintptr_t>()) = reinterpret_cast<uintptr_t>(p);
}

void Value::set_parent(std::shared_ptr<Value> p) { parent_ = std::move(p); }

NullValue::NullValue(std::shared_ptr<Value> p) {
  set_parent(std::move(p));
  init_tagged(Tag::kNull);
}

BooleanValue::BooleanValue(std::shared_ptr<Value> p, bool b) {
  set_parent(std::move(p));
  init_tagged(Tag::kBoolean);
  *(cast_data<bool>()) = b;
}

Int32Value::Int32Value(std::shared_ptr<Value> p, int32_t i) {
  set_parent(std::move(p));
  init_tagged(Tag::kInt32);
  *(cast_data<int32_t>()) = i;
}

Float32Value::Float32Value(std::shared_ptr<Value> p, float f) {
  set_parent(std::move(p));
  init_tagged(Tag::kFloat32);
  *(cast_data<float>()) = f;
}

} // namespace dom
