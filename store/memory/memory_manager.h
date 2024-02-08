#pragma once
#include "arena_alloc.h"

namespace mem {

class MemoryManager {
public:
  explicit MemoryManager(int fd);
  ~MemoryManager();
  ArenaAlloc newArenaAllocator();
};

} // namespace mem
