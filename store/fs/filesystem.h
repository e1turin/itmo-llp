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
    explicit Offset(const long int n) : m_offset(n) {}

    [[nodiscard]] long int value() const { return m_offset; }

  private:
    const long int m_offset;
  };

  enum class Mode : uint8_t {
    kReadOnly  = 0b0000,
    kReadWrite = 0b0001,
  };

  explicit File(const Handle h, const Mode m)
      : handle_(std::move(h)), mode_(m){};

  [[nodiscard]] Handle handle() const { return handle_; }
  [[nodiscard]] Mode mode() const { return mode_; }

  void close();

  ~File();

private:
  const Handle handle_;
  const Mode mode_;
};

class FileManager {
public:
  FileManager();

  [[nodiscard]] static std::shared_ptr<File> open(std::string_view);

private:
  std::vector<std::shared_ptr<File>> m_files;
};

} // namespace fs
