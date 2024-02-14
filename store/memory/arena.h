#pragma once
#include <store/fs/filesystem.h>

namespace mem {

class Arena {
public:
  Arena(const fs::Offset addr, const size_t size) : addr_(addr), size_(size) {}

private:
  fs::Offset addr_;
  size_t size_;
};

class ArenaAlloc {
public:
  explicit ArenaAlloc(LPVOID, size_t);
  bool reset(LPVOID);
  Arena *alloc(size_t);
  bool free(Arena *);
private:
  LPVOID file_view_begin_;
  size_t view_size_;
  Arena *free_list_;
};

} // namespace mem