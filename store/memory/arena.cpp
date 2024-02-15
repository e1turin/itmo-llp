#include "arena.h"

namespace mem {

ArenaAlloc::ArenaAlloc(LPVOID begin, size_t size)
    : file_view_begin_(begin), view_size_(size) {}

bool ArenaAlloc::reset(LPVOID) {}

Arena *ArenaAlloc::alloc(size_t) {}

bool ArenaAlloc::free(Arena *) {}
} // namespace mem
