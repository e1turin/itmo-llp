#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <windows.h>

namespace fs {

/**
 * platform specific file handle: HANDLE on windows, file descriptor on linux
 */
class Handle {
  HANDLE handle_;

public:
  explicit Handle(HANDLE h) : handle_(h) {}

  HANDLE handle() { return handle_; }
};

/**
 * Platform specific file offset type.
 */
class Offset {
public:
  explicit Offset(const long int n) : offset_(n) {}

  [[nodiscard]] long int value() const { return offset_; }

private:
  const long int offset_;
};

/*
  enum Mode {
    kReadOnly,
    kReadWrite,
  };
*/

class File {
public:
  [[nodiscard]] Handle handle() const { return handle_; }

private:
  friend class FileManager;

  explicit File(const Handle h) : handle_(h){};

  ~File() = default;

  const Handle handle_;
};

class FileManager {
public:
  [[nodiscard]] const File *open(const std::string_view &);
  bool close(const File *);

private:
  std::vector<File *> files_;
};

} // namespace fs
