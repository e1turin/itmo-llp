#ifndef ITMO_LLP_STORE_DOM_NODE_H_
#define ITMO_LLP_STORE_DOM_NODE_H_

#include <cstdint>
#include <string>

namespace dom {
class alignas(8) Value {
 public:
  enum class Type {
    kInt32,
    kFloat64,
    kString,
    kBoolean,
    kObject,
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

 protected:
  //
  enum class Tag : uint8_t {
    kShortString = 0b0000'0000,  // inline payload
    kInt32       = 0b0000'0001,  // inline payload
    kFloat32     = 0b0000'0010,  // inline payload
    kBoolean     = 0b0000'0011,  // inline payload
    kObject      = 0b0000'0100,  // offset to store place
    kString      = 0b0000'0101,  // offset to store place
  };
  inline static constexpr uint8_t kTagMask = 0b0000'0111;

  /* Value (like in Skia: skjson::Value):
   * +------------------------------------------------------------+
   * | Tag |      remaining data (inline/offset by 64 bit)        |
   * +------------------------------------------------------------+
   */

 private:
  inline static constexpr size_t kValueSize = 8;  // Value type is 64 bit size and lower bits are used for tag.
                                                  // For referencing data of size bigger than 64 bit remaining 61 bit
                                                  // can be used after excluding tag value (addressing by 8 byte).
  uint8_t fData[kValueSize];
};

class Int32Value final : Value {
  inline static constexpr Type type = Type::kInt32;

  explicit Int32Value(int32_t);
};

class BoolValue final : Value {
  inline static constexpr Type type = Type::kBoolean;

  explicit BoolValue(bool);
};

class Float64Value final : Value {
  inline static constexpr Type type = Type::kFloat64;

  explicit Float64Value(double);
};

class StringValue final : Value {
  inline static constexpr Type type = Type::kString;

  explicit StringValue(std::string);
};

struct Member {
  StringValue fKey;
  Value fValue;
};

class ObjectValue final : Value {
  inline static constexpr Type type = Type::kObject;

  //
};

// TODO

}  // namespace dom

#endif  // ITMO_LLP_STORE_DOM_NODE_H_
