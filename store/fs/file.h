#pragma once

#include <string_view>
#include <windows.h>

namespace fs {

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
