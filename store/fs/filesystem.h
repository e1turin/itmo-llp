#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <windows.h>

namespace fs {

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
  [[nodiscard]] HANDLE handle() const { return handle_; }

private:
  friend class FileManager;

  HANDLE handle_;
  const std::string_view name_;

  explicit File(HANDLE h, const std::string_view name)
      : handle_(h), name_(name){};

  ~File() = default;
};

class FileManager {
public:
  [[nodiscard]] const File *open(const std::string_view &);

  bool close(const File *);

  ~FileManager();

private:
  std::vector<File *> files_;
};

} // namespace fs
