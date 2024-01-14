
#include "node.h"

namespace dom {

void Value::init_tagged(Tag t) {
  this->tag = t;
  memset(payload, 0, sizeof(payload));
}

void Value::init_tagged_pointer(const Tag t, void* p) {
  this->init_tagged(t);
  *(this->cast_data<uintptr_t>()) = reinterpret_cast<uintptr_t>(p);
}

void Value::set_parent(std::shared_ptr<Value> p) {
  this->parent = std::move(p);
}

NullValue::NullValue(std::shared_ptr<Value> p) {
  this->set_parent(std::move(p));
  this->init_tagged(Tag::kNull);
}

BooleanValue::BooleanValue(std::shared_ptr<Value> p, bool b) {
  this->set_parent(std::move(p));
  this->init_tagged(Tag::kBoolean);
  *(this->cast_data<bool>()) = b;
}

Int32Value::Int32Value(std::shared_ptr<Value> p, int32_t i) {
  this->set_parent(std::move(p));
  this->init_tagged(Tag::kInt32);
  *(this->cast_data<int32_t>()) = i;
}

Float32Value::Float32Value(std::shared_ptr<Value> p, float f) {
  this->set_parent(std::move(p));
  this->init_tagged(Tag::kFloat32);
  *(this->cast_data<float>()) = f;
}

}  // namespace types
