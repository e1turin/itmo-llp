#ifndef ITMO_LLP_STORE_DOM_NODE_H_
#define ITMO_LLP_STORE_DOM_NODE_H_

#include "cstdint"

namespace dom {
class Value {
 public:
  enum class Type {
    Int32,
    Float64,
    String,
    Boolean,
    Object,
  };

  [[nodiscard]] Type getType() const;

  template <typename T>
  [[nodiscard]] bool is() const {
    return this->getType() == T::Type;
  }

  template <typename T>
  [[nodiscard]] T& as() const {
    static_assert(this->is<T>());
    return *reinterpret_cast<const T*>(this);
  }
};

class Int32Value final : Value {
  inline static constexpr Type type = Type::Int32;

  explicit Int32Value(int32_t);
};

class BoolValue final : Value {
  inline static constexpr Type type = Type::Boolean;

  explicit BoolValue(bool);
};

// TODO

}  // namespace dom

#endif  // ITMO_LLP_STORE_DOM_NODE_H_
