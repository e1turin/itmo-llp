#pragma once
#include <utility>

#include "arena.h"
#include "segment.h"
#include "store/fs/filesystem.h"

namespace mem {

class MemoryManager {
public:
  explicit MemoryManager(std::shared_ptr<fs::FileManager> fm)
      : file_manager_(std::move(fm)) {}

  [[nodiscard]] std::shared_ptr<ArenaAlloc> create_arena_allocator();

  std::shared_ptr<Segment> read(std::shared_ptr<fs::File> &, fs::File::Offset,
                                size_t);

  ~MemoryManager();

private:
  std::shared_ptr<fs::FileManager> file_manager_;
  std::vector<std::shared_ptr<Segment>> segments_;

  [[nodiscard]] std::shared_ptr<SegmentAlloc> create_segment_alloc();
};

} // namespace mem
