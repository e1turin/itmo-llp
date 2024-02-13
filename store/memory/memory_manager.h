#pragma once
#include <utility>

#include "arena.h"
#include "segment.h"
#include "store/fs/filesystem.h"
#include <map>

namespace mem {

class MemoryManager {
public:
  explicit MemoryManager(std::unique_ptr<fs::FileManager> fm)
      : file_manager_(std::move(fm)) {}

  [[nodiscard]] std::shared_ptr<ArenaAlloc> create_arena_allocator();

  std::shared_ptr<Segment> read(std::shared_ptr<fs::File> &, fs::Offset,
                                size_t);

  ~MemoryManager();

private:
  std::unique_ptr<fs::FileManager> file_manager_;
  std::map<fs::File *, std::shared_ptr<SegmentAlloc>> seg_allocs_;

  [[nodiscard]] std::shared_ptr<SegmentAlloc> create_segment_alloc();
};

} // namespace mem
