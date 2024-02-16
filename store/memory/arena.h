#pragma once
#include <store/fs/file.h>
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
  explicit ArenaAlloc(LPVOID, size_t);
  bool reset(std::byte *);
  Arena *alloc(size_t);
  bool free(Arena *);
private:
  void *file_view_begin_;
  size_t view_size_;
  std::vector<Arena> free_begin_arenas_;
};

} // namespace mem