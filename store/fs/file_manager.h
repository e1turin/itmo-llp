#pragma once

#include <store/memory/mmap_manager.h>

#include <memory>
#include <string_view>

namespace fs {

class File {
 public:
  explicit File(std::string_view);
  [[nodiscard]] mem::MmapManager MapMemory();
  ~File();
private:
};

class FileManager {
 public:
  FileManager();
  [[nodiscard]] std::unique_ptr<File> Open(std::string_view);
};

}  // namespace fs
