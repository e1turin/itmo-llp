#pragma once

#include "file_layout.h"
#include "store/fs/file.h"

#include <vector>

namespace mem {


class ArenaAlloc final {
public:
  explicit ArenaAlloc(HANDLE, size_t);
  ~ArenaAlloc();

  [[nodiscard]] dom::Value get_root() const;
  [[nodiscard]] std::byte *data_ptr() const;
  /**
   * Allocates arena of memory where can be stored `size` of bytes.
   * @param size Amount of bytes to allocate.
   * @param lack Amount of bytes that are not enough to allocate required `size`.
   * @return Pointer to memory where can be writen.
   */
  Offset alloc(size_t);
  size_t free(Offset);
  bool is_valid(Offset);

private:
  bool map_memory();
  bool unmap_memory();
  static bool is_null(Offset);
  Arena *arena_from(Offset);

  HANDLE file_map_obj_;
  MemView mem_view_ = {nullptr};
  size_t size_;
};

} // namespace mem