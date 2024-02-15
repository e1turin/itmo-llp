#pragma once

#include <memory>
#include <string_view>
#include <windows.h>

namespace fs {

/**
 * Platform specific file offset type.
 */
class Offset {
public:
  explicit Offset(const size_t n) : offset_(n) {}

  [[nodiscard]] size_t value() const { return offset_; }

private:
  const size_t offset_;
};


class File {
public:
  [[nodiscard]] HANDLE handle() const { return handle_; }

  explicit File(std::string_view name);

  ~File();

private:
  HANDLE handle_;
  const std::string_view name_;
};

} // namespace fs
