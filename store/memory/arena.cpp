#include "arena.h"

namespace mem {

ArenaAlloc::ArenaAlloc(std::byte *begin, size_t size)
    : file_view_begin_(begin), view_size_(size) {}

} // namespace mem
