#include "segment.h"

namespace mem {

std::unique_ptr<Segment> mem::SegmentAlloc::alloc(size_t size) {}

std::unique_ptr<Segment> mem::SegmentAlloc::alloc(fs::File::Offset, size_t size,
                                                  fs::File::Mode) {}

size_t SegmentAlloc::free(std::unique_ptr<Segment>) {}

} // namespace mem
