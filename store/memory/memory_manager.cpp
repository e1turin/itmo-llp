#include "memory_manager.h"
#include "store/fs/filesystem.h"

namespace mem {

std::shared_ptr<ArenaAlloc> MemoryManager::create_arena_allocator() {}

std::shared_ptr<SegmentAlloc> MemoryManager::create_segment_alloc() {}

MemoryManager::~MemoryManager() {}

std::shared_ptr<Segment> MemoryManager::read(std::shared_ptr<fs::File> &,
                                             fs::File::Offset, size_t) {}

} // namespace mem
