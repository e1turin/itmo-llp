#pragma once
#include "file_layout.h"
#include "store/fs/file.h"
#include <vector>

namespace mem {


class ArenaAlloc final {
public:
  using MetaInfo = FileHeader *;

  ArenaAlloc(MetaInfo meta_info, std::byte *data)
      : meta_info_(meta_info), data_(data) {}

  /**
   * Allocates arena of memory where can be stored `size` of bytes.
   * @param size Amount of bytes to allocate.
   * @param lack Amount of bytes that are not enough to allocate required `size`.
   * @return Pointer to memory where can be writen.
   */
  Arena *alloc(size_t, size_t &);
  bool free(Arena *);
  bool is_valid(fs::Offset);
  static bool is_null(fs::Offset);

private:
  MetaInfo meta_info_;
  std::byte *data_;
};

} // namespace mem