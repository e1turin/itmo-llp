#pragma once
#include "file_layout.h"
#include "store/fs/file.h"
#include <vector>

namespace mem {

class Arena {
public:
  Arena(const fs::Offset addr, const size_t size) : addr_(addr), size_(size) {}

private:
  fs::Offset addr_;
  size_t size_;
};

class ArenaAlloc final {
public:
  explicit ArenaAlloc(std::byte *, size_t);
  std::byte *alloc(size_t);
  bool free(Arena *);
  bool reload(std::byte *);

private:
  std::byte *file_view_begin_;
  size_t view_size_;
  std::vector<Arena> free_begin_arenas_;
};

} // namespace mem