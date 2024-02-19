#pragma once
#include "store/dom/value.h"
#include "store/fs/file.h"

constexpr size_t calc_num(size_t min, size_t max) {
  size_t c = 0;
  for (size_t s = min; s <= max; s *= 2) {
    c++;
  }
  return c;
}

namespace mem {

struct FileHeader {
  dom::Value root;
  fs::Offset last_arena;


  static constexpr size_t kMinArenaSizeInBytes = 64;
  static constexpr size_t kMaxArenaSizeInBytes = 8LL * (1 << 30); // 8GB
  static constexpr size_t kAvailableSizes =
      calc_num(kMaxArenaSizeInBytes, kMaxArenaSizeInBytes);

  fs::Offset free_fixed_arena[kAvailableSizes];
  fs::Offset free_ext_arena;
};

} // namespace mem
