#pragma once

#include "store/fs/file.h"
#include "store/memory/offset.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace dom {

class alignas(8) Value {
public:
  enum class Type { kBoolean, kNull, kInt32, kFloat32, kString, kObject };

public:
  [[nodiscard]]
  Type get_type() const;
  [[nodiscard]] bool is_primitive() const;

  template <typename T>
  [[nodiscard]]
  bool is() const {
    return get_type() == T::kType;
  }

  template <typename T>
  const T &as() const {
    return *reinterpret_cast<const T *>(this);
  }

protected:
  enum class Tag : uint8_t {
    /* Tag can be inlined into node data: 3 bits are enough for representing any
       available type and remaining 61 bits is enough for addressing Value
       objects aligned as 64 bit. */

    // Binary values can be used instead, but now I am mot saving memory.
    kNull = 'N', // = 0b0000'0000 // no payload, must be replaced by obj with 0
                 // reference
    kShortString = 'H', // = 0b0000'0001 // inline value, unused for now
    kBoolean     = 'B', // = 0b0000'0010 // inline value
    kInt32       = 'I', // = 0b0000'0011 // inline value
    kFloat32     = 'F', // = 0b0000'0100 // inline value
    kString      = 'S', // = 0b0000'0101 // reference
    kObject      = 'O', // = 0b0000'0110 // reference
  };
  static constexpr uint8_t kTagMask = 0b0000'0111;

  void init_tagged(Tag);
  void init_tagged_pointer(Tag, void *);

  template <typename T>
  [[nodiscard]]
  const T *cast_data() const {
    return reinterpret_cast<const T *>(payload_);
  }

  template <typename T>
  T *cast_data() {
    return const_cast<T *>(const_cast<const Value *>(this)->cast_data<T>());
  }

private:
  Tag tag_ = Tag::kNull;

  static constexpr size_t kValueSize = 8;
  std::byte payload_[kValueSize];
};

class NullValue final : public Value {
public:
  static constexpr Type kType = Type::kNull;

  explicit NullValue();
};

class BoolValue final : public Value {
public:
  static constexpr Type kType = Type::kBoolean;

  explicit BoolValue(bool);

  [[nodiscard]]
  bool get_bool() const;
};

class Int32Value final : public Value {
public:
  static constexpr Type kType = Type::kInt32;

  explicit Int32Value(std::int32_t);

  [[nodiscard]]
  std::int32_t get_int() const;
};

class Float32Value final : public Value {
public:
  static constexpr Type kType = Type::kFloat32;

  explicit Float32Value(float);

  [[nodiscard]]
  float get_float() const;
};

/**
 * Additional class for vector-like types,
 * `vtype` template parameter is for future inline-string implementation
 */
template <typename T, Value::Type vtype>
class VectorValue : public Value {
public:
  static constexpr Type kType = vtype;

  [[nodiscard]]
  mem::Offset get_ref() const {
    // NOTE: need to clear inline tag
    return mem::Offset{*cast_data<size_t>()};
  }
};

/**
 *  Every string is non-inline for now.
 */
class StringValue final : public VectorValue<char, Value::Type::kString> {
public:
  explicit StringValue(mem::Offset);
};

struct Entry {
  StringValue key;
  Value value;
};

class ObjectValue final : public VectorValue<Entry, Value::Type::kObject> {
public:
  explicit ObjectValue(mem::Offset);
  static ObjectValue null_object() { return ObjectValue(); }

private:
  explicit ObjectValue();
};

inline Value::Type Value::get_type() const {
  switch (tag_) {
  case Tag::kShortString: return Type::kString;
  case Tag::kBoolean:     return Type::kBoolean;
  case Tag::kInt32:       return Type::kInt32;
  case Tag::kFloat32:     return Type::kFloat32;
  case Tag::kString:      return Type::kString;
  case Tag::kObject:      return Type::kObject;
  case Tag::kNull:
  default:                return Type::kNull;
  }
}

inline bool Value::is_primitive() const {
  switch (tag_) {
  case Tag::kBoolean:
  case Tag::kInt32:
  case Tag::kFloat32:
  case Tag::kShortString: return true;
  case Tag::kString:
  case Tag::kObject:
  case Tag::kNull:
  default:                return false;
  }
}

} // namespace dom
