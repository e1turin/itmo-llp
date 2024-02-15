#pragma once

#include "store/fs/file.h"
#include <store/dom/value.h>

namespace mem {

class MemoryManager final {
public:
  explicit MemoryManager(fs::File &&);

  ~MemoryManager();

  template <typename V>
  V read(fs::Offset);

  template <typename V>
  bool write(fs::Offset, V);

  [[nodiscard]] fs::Offset alloc(size_t) const;

  [[nodiscard]] size_t free(fs::Offset, size_t) const;

private:
  fs::File file_;
  HANDLE file_map_obj_;
  std::byte *file_view_begin_;

  [[nodiscard]] void *address_of(fs::Offset) const;

  bool remap_file();
};

template <typename V>
V MemoryManager::read(fs::Offset) {}

template <typename V>
bool MemoryManager::write(fs::Offset, V) {}

} // namespace mem
