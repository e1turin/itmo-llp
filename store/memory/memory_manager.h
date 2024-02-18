#pragma once

#include <optional>
#include <store/dom/value.h>
#include <store/fs/file.h>
#include <util/util.h>
#include <store/memory/arena.h>

namespace mem {

class MemoryManager final {
public:
  explicit MemoryManager(fs::File &&);

  ~MemoryManager();

  template <typename T>
  [[nodiscard]]
  std::optional<T> read(fs::Offset offset) const {
    if (is_valid(offset)) {
      return *reinterpret_cast<T *>(address_of(offset));
    } else {
      return std::nullopt;
    }
  }

  template <typename T>
  [[nodiscard]]
  std::optional<std::vector<T>> read_all(fs::Offset offset) const {
    if (!is_valid(offset)) {
      return std::nullopt;
    }
    auto size = reinterpret_cast<size_t *>(address_of(offset));
    T *begin   = reinterpret_cast<T *>(size + 1);
    T *end     = begin + *size;
    return std::vector<T>{begin, end};
  }

  bool write(fs::Offset, std::byte *, std::byte *);
  [[nodiscard]] fs::Offset alloc(size_t) const;
  [[nodiscard]] size_t free(fs::Offset, size_t) const;

private:
  fs::File file_;
  HANDLE file_map_obj_;
  std::byte *file_view_begin_;
  std::unique_ptr<mem::ArenaAlloc> alloc_;

  [[nodiscard]] std::byte *address_of(fs::Offset) const;
  [[nodiscard]] bool is_valid(fs::Offset) const;

  bool remap_file(size_t);
};

} // namespace mem
