#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <windows.h>

namespace fs {

class File {
public:
  /**
   * platform specific file handle: HANDLE on windows, file descriptor on linux
   */
  class Handle {
    const HANDLE handle_;

  public:
    explicit Handle(const HANDLE h) : handle_(h) {}

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

  enum Mode {
    kReadOnly,
    kReadWrite,
  };

  [[nodiscard]] Handle handle() const { return handle_; }
  [[nodiscard]] Mode mode() const { return mode_; }


private:
  friend class FileManager;

  explicit File(const Handle h, const Mode m)
      : handle_(h), mode_(m){};

  ~File() = default;

  const Handle handle_;
  const Mode mode_;
};

class FileManager {
public:
  [[nodiscard]] const File *open(const std::string_view &, File::Mode);
  bool close(const File *);

private:
  std::vector<File *> files_;
};

} // namespace fs
