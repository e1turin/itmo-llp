#pragma once

namespace mem {

/**
 * Platform specific file offset type.
 */
class Offset {
public:
  explicit Offset(const size_t n) : offset_(n) {}

  [[nodiscard]]
  size_t value() const {
    return offset_;
  }

  template <typename T>
  [[nodiscard]]
  Offset after() {
    return Offset{offset_ + sizeof(T)};
  }

  template <typename T>
  [[nodiscard]]
  Offset before() {
    return Offset{offset_ - sizeof(T)};
  }

private:
  size_t offset_;
};

} // namespace mem
