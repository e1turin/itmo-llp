#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "store/memory/arena_alloc.h"

namespace dom {

class alignas(8) Value {
public:
  enum class Type { kBoolean, kNull, kInt32, kFloat32, kString, kObject };

  [[nodiscard]] Type getType() const;

  template <typename T>
  [[nodiscard]] bool is() const {
    return getType() == T::kType;
  }

  template <typename T>
  const T &as() const {
    static_assert(this->is<T>());
    return *reinterpret_cast<const T *>(this);
  }

protected:
  enum class Tag : uint8_t {
    // Tag can be inlined into node data: 3 bits are enough for representing any
    // available type and remaining 61 bits is enough for addressing Value
    // objects aligned as 64 bit.

    // Binary values can be used instead, but now I don't save memory.
    kShortString = 'H', // = 0b0000'0000 // inline value // unused
    kNull        = 'N', // = 0b0000'0001 // no payload
    kBoolean     = 'B', // = 0b0000'0010 // inline value
    kInt32       = 'I', // = 0b0000'0011 // inline value
    kFloat32     = 'F', // = 0b0000'0100 // inline value
    kString      = 'S', // = 0b0000'0101 // reference
    kObject      = 'O', // = 0b0000'0110 // reference
  };
  inline static constexpr uint8_t kTagMask = 0b0000'0111; // unused

  void init_tagged(Tag);
  void init_tagged_pointer(Tag, void *);
  void set_parent(std::shared_ptr<Value>);

  template <typename T>
  [[nodiscard]] const T *data_ptr() const {
    // maybe used cast_data with offset
    return reinterpret_cast<const T *>(payload_);
  }

  template <typename T>
  const T *cast_data() const {
    return reinterpret_cast<const T *>(payload_);
  }

  template <typename T>
  T *cast_data() {
    return const_cast<T *>(const_cast<const Value *>(this)->cast_data<T>());
  }

private:
  std::shared_ptr<Value> parent_;
  Tag tag_ = Tag::kNull;

  static constexpr size_t kValueSize = 8;
  uint8_t payload_[kValueSize];
};

class NullValue final : Value {
public:
  NullValue(Value *p);
  static constexpr Type kType = Type::kNull;

  NullValue(std::shared_ptr<Value>);
};

class BooleanValue final : Value {
public:
  static constexpr Type kType = Type::kBoolean;

  BooleanValue(std::shared_ptr<Value>, bool);
};

class Int32Value final : Value {
public:
  static constexpr Type kType = Type::kInt32;

  Int32Value(std::shared_ptr<Value>, int32_t);
};

class Float32Value final : Value {
public:
  static constexpr Type kType = Type::kFloat32;

  Float32Value(std::shared_ptr<Value>, float);
};

// additional class for vector-like types
template <typename T, Value::Type vtype>
class VectorValue : public Value {
public:
  static constexpr Type kType = vtype;

  [[nodiscard]] size_t size() const { return *data_ptr<size_t>(); }

  [[nodiscard]] const T *begin() const {
    const auto size_ptr = data_ptr<size_t>();
    return reinterpret_cast<const T *>(size_ptr + 1);
  }

  [[nodiscard]] const T *end() const {
    const auto size_ptr = data_ptr<size_t>();
    return reinterpret_cast<const T *>(size_ptr + 1) + *size_ptr;
  }

  const T &operator[](size_t i) const { return *(begin() + i); }
};

// every string is non-inline for now
class StringValue final : public VectorValue<char, Value::Type::kString> {
public:
  StringValue(std::shared_ptr<Value>, std::string, mem::ArenaAlloc &);

  [[nodiscard]] std::string_view str() const {
    return std::string_view{begin(), size()};
  }
};

struct Entry {
  StringValue key;
  Value value;
};

class ObjectValue final : VectorValue<Entry, Value::Type::kObject> {
public:
  explicit ObjectValue(std::shared_ptr<Value>);
  ObjectValue(std::shared_ptr<Value>, std::vector<Entry>, mem::ArenaAlloc &);

  const Value &operator[](std::string) const;
  const Entry &operator[](size_t i) const { return VectorValue::operator[](i); }
};

inline Value::Type Value::getType() const {
  switch (tag_) {
  case Tag::kShortString: return Type::kString;
  case Tag::kBoolean:     return Type::kBoolean;
  case Tag::kInt32:       return Type::kInt32;
  case Tag::kFloat32:     return Type::kFloat32;
  case Tag::kString:      return Type::kString;
  case Tag::kObject:      return Type::kObject;
  case Tag::kNull:        return Type::kNull;
  } // unreachable
  return Type::kNull;
}

} // namespace dom
