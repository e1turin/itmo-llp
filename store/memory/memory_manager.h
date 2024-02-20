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

  template<typename T>
  [[nodiscard]]
  std::optional<T> read_root() const {
    auto *header = reinterpret_cast<FileHeader *>(file_view_.header);
    return std::optional<T>(header->root);
  }

  template <typename T>
  [[nodiscard]]
  std::optional<T> read(fs::Offset offset) const;

  template <typename T>
  [[nodiscard]]
  std::optional<std::vector<T>> read_all(fs::Offset offset) const;

  /**
   * @param dest
   * @param begin
   * @param end
   * @return true if OK
   */
  bool write(fs::Offset, std::byte *, std::byte *);
  [[nodiscard]] fs::Offset alloc(size_t) const;
  [[nodiscard]] size_t free(fs::Offset, size_t) const;

private:
  fs::File file_;
  HANDLE file_map_obj_;
  FileView file_view_;
  std::unique_ptr<mem::ArenaAlloc> alloc_;

  [[nodiscard]] std::byte *address_of(fs::Offset) const;
  [[nodiscard]] bool is_valid(fs::Offset) const;

  bool remap_file(size_t);
};

template <typename T>
std::optional<T> MemoryManager::read(fs::Offset offset) const {
  if (is_valid(offset)) {
    return *reinterpret_cast<T *>(address_of(offset));
  } else {
    return std::nullopt;
  }
}

template <typename T>
std::optional<std::vector<T>> MemoryManager::read_all(fs::Offset offset) const {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  T *begin   = reinterpret_cast<T *>(size + 1);
  T *end     = begin + *size;
  return std::vector<T>{begin, end};
}

} // namespace mem
