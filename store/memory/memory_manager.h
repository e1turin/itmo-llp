#pragma once

#include "file_layout.h"
#include "store/dom/value.h"
#include "store/fs/file.h"
#include "store/memory/arena.h"
#include "util/util.h"

#include <optional>

namespace mem {

class MemoryManager final {
public:
  explicit MemoryManager(fs::File &&);

  [[nodiscard]]
  std::optional<dom::Value> get_root() const;

  template <Derived<dom::Value> T>
  [[nodiscard]]
  std::optional<T> read(Offset offset) const;

  template <typename T>
  [[nodiscard]]
  std::optional<std::vector<T>> read_all(Offset offset) const;

  /**
   * @param dest
   * @param begin
   * @param end
   * @return true if OK
   */
  bool write(Offset, const std::byte *, const std::byte *);
  [[nodiscard]] Offset alloc(size_t) const;
  [[nodiscard]] size_t free(Offset) const;

private:
  [[nodiscard]] std::byte *address_of(Offset) const;
  [[nodiscard]] bool is_valid(Offset) const;

  fs::File file_;
  std::unique_ptr<mem::ArenaAlloc> alloc_;
};

template <Derived<dom::Value> T>
std::optional<T> MemoryManager::read(Offset offset) const {
  if (is_valid(offset)) {
    return *reinterpret_cast<T *>(address_of(offset));
  } else {
    return std::nullopt;
  }
}

template <typename T>
std::optional<std::vector<T>> MemoryManager::read_all(Offset offset) const {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  T *begin  = reinterpret_cast<T *>(size + 1);
  T *end    = begin + *size;
  return std::vector<T>{begin, end};
}

} // namespace mem
