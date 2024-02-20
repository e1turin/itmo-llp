#pragma once
#include "store/dom/value.h"
#include "store/fs/file.h"



namespace mem {

constexpr size_t calc_num(size_t min, size_t max) {
  size_t c = 0;
  for (size_t s = min; s <= max; s *= 2) {
    c++;
  }
  return c;
}
constexpr size_t kMinArenaSizeInBytes = 64;
constexpr size_t kMaxArenaSizeInBytes = 4LL * (1 << 30); // 4GB
constexpr size_t kNumAvailableSizes =
    calc_num(kMinArenaSizeInBytes, kMaxArenaSizeInBytes);

struct FileHeader {
  std::uint64_t magic; /* some magic number for any purpose */
  dom::Value root; /* single value-object, ref to block of another values */
  fs::Offset free_fixed_arena[kNumAvailableSizes]; /* array of refs to*/
  fs::Offset free_ext_arena;
  fs::Offset last_arena; /* ref to the furthest allocated arena */
};

union FileView {
  LPVOID view_ptr;
  std::byte *data;
  FileHeader *header;
};

struct Arena {
  size_t size;
  std::byte data[];
};

size_t fit_arena_idx(size_t size) {
  size_t idx = 0; // smallest size
  while(size > kMinArenaSizeInBytes && idx < kNumAvailableSizes) {
    size >>= 1;
    idx++;
  }
  return idx;
}

} // namespace mem
