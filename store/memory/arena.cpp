#include "arena.h"

namespace mem {

Arena *ArenaAlloc::alloc(size_t size, size_t &lack) {
  size_t arena_size_idx = fit_arena_idx(size);
  fs::Offset offset{0};
  if (arena_size_idx < kNumAvailableSizes) {
    offset = meta_info_->free_fixed_arena[arena_size_idx];
    while (is_null(offset) && arena_size_idx < kNumAvailableSizes) {
      // TODO: split in 2 smaller arenas
      offset = meta_info_->free_fixed_arena[++arena_size_idx];
    }
    if (is_null(offset)) {
      offset = meta_info_->free_ext_arena;
    }
  } else {
    offset = meta_info_->free_ext_arena;
  }
  if (!is_valid(offset)){
    lack = size;
    return nullptr;
  }
  auto arena = reinterpret_cast<Arena *>(data_ + offset.value());
  return arena;
}

bool ArenaAlloc::is_valid(fs::Offset offset) {
  //TODO: consider alignment & referred data content tag
  if (offset.value() == 0){
    return false;
  }
  return true;
}
bool ArenaAlloc::is_null(fs::Offset offset) { return offset.value() == 0; }

} // namespace mem
