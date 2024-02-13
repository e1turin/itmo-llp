#include "segment.h"

namespace mem {

std::unique_ptr<Segment> SegmentAlloc::alloc(size_t size) {}

std::unique_ptr<Segment> SegmentAlloc::alloc(fs::Offset, size_t size) {}

size_t SegmentAlloc::free(std::unique_ptr<Segment>) {}

} // namespace mem
