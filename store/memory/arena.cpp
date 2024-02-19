#include "arena.h"

namespace mem {

std::byte *ArenaAlloc::alloc(size_t size, size_t &lack) {
  fs::Offset offset = meta_info_->free_fixed_arena[fit_arena_idx(size)];
  auto *arena       = reinterpret_cast<Arena *>(data_ + offset.value());
  //...
}

} // namespace mem
