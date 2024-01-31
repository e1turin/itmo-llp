#pragma once
#include "arena_alloc.h"

namespace mem {

class MmapManager {
 public:
  explicit MmapManager(int fd);
  ~MmapManager();
  ArenaAlloc newArenaAllocator();
};

}  // namespace mem
