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
  Offset after(size_t num = 1) {
    return Offset{offset_ + num * sizeof(T)};
  }

  template <typename T>
  [[nodiscard]]
  Offset before(size_t num = 1) {
    return Offset{offset_ - num * sizeof(T)};
  }

private:
  size_t offset_;
};

} // namespace mem
