
#include "arena.h"

namespace mem {

std::unique_ptr<Arena> ArenaAlloc::alloc(size_t) {}

size_t ArenaAlloc::free(std::unique_ptr<Arena>) {}

uint8_t Arena::get(size_t) {}

void Arena::set(size_t, uint8_t) {}

} // namespace mem
