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

private:
  size_t offset_;
};

} // namespace mem
