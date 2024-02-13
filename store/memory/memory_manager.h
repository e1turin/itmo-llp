#pragma once
#include <utility>

#include "store/fs/filesystem.h"
#include <map>
#include <store/dom/node.h>

namespace mem {

class MemoryManager {
public:
  explicit MemoryManager(std::unique_ptr<fs::FileManager>, std::string_view);

  ~MemoryManager();

  template <typename V>
  V read(fs::Offset);

  template <typename V>
  bool write(fs::Offset, V);

  fs::Offset alloc(size_t);

  size_t free(fs::Offset, size_t);

private:
  std::unique_ptr<fs::FileManager> file_manager_;
  const fs::File *file_;
  HANDLE file_map_obj_;
  LPVOID file_view_begin_;
};

template <typename V>
V MemoryManager::read(fs::Offset) {}

template <typename V>
bool MemoryManager::write(fs::Offset, V) {}

} // namespace mem
