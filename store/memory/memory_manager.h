#pragma once

#include "file_layout.h"
#include "offset.h"
#include "store/dom/value.h"
#include "store/fs/file.h"
#include "util/util.h"

#include <functional>
#include <optional>

namespace mem {

class MemoryManager final {
public:
  explicit MemoryManager(fs::File &&);

  [[nodiscard]]
  std::optional<mem::Offset> root_ref() const;

  template <Derived<dom::Value> T>
  [[nodiscard]]
  std::optional<T> read(Offset offset) const;
  template <typename T>
  [[nodiscard]] std::optional<std::vector<T>> read_all(Offset) const;
  template <typename T>
  [[nodiscard]] std::optional<std::vector<std::pair<Offset, Offset>>>
      ref_all_pairs(Offset) const;

  template <typename T, typename R>
  std::optional<R> do_for_entries(Offset offset,
                                  std::function<R(size_t, T *)> func) {
    if (!is_valid(offset)) {
      return std::nullopt;
    }
    auto size = reinterpret_cast<size_t *>(address_of(offset));
    auto data = reinterpret_cast<T *>(size + 1);
    return std::optional{func(*size, data)};
  }
  template <typename T, typename R>
  std::optional<Offset> find_in_entries(Offset,
                                        std::function<R *(size_t, T *)>);

  /**
   * @param dest
   * @param begin
   * @param end
   * @return true if OK
   */
  bool write(Offset, const void *, size_t);
  template <Derived<dom::Value> T>
  bool write(Offset offset, T &&value);

  template <typename T>
  [[nodiscard]]
  Offset alloc(size_t size);
  [[nodiscard]] size_t free(Offset) const;

private:
  [[nodiscard]] std::byte *expand_file_by(size_t);
  void map_file();
  [[nodiscard]] Arena *arena_for(Offset) const;
  [[nodiscard]] Offset data_offset_in(Arena *) const;
  [[nodiscard]] Offset use_arena(Offset) const;
  [[nodiscard]] std::byte *address_of(Offset) const;
  [[nodiscard]] Offset offset_of(void *) const;
  [[nodiscard]] bool is_valid(Offset) const;
  [[nodiscard]] static bool is_null(Offset);

  fs::File file_;
  size_t file_size_;
  HANDLE file_map_obj_;
  MemView mem_view_;
};

template <Derived<dom::Value> T>
std::optional<T> MemoryManager::read(Offset offset) const {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  return *reinterpret_cast<T *>(address_of(offset));
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

template <typename T>
std::optional<std::vector<std::pair<Offset, Offset>>>
MemoryManager::ref_all_pairs(Offset offset) const {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  auto data  = reinterpret_cast<std::pair<T, T> *>(size + 1);
  std::vector<std::pair<Offset, Offset>> pairs;
  pairs.reserve(*size);
  for(size_t i = 0; i < *size; ++i) {
    pairs.emplace_back(offset_of(&data[i].first), offset_of(&data[i].second));
  }
  return pairs;
}


template <typename T, typename R>
std::optional<Offset>
MemoryManager::find_in_entries(Offset offset,
                               std::function<R *(size_t, T *)> func) {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size      = reinterpret_cast<size_t *>(address_of(offset));
  auto data      = reinterpret_cast<T *>(size + 1);
  auto value_ptr = func(*size, data);
  if (value_ptr == nullptr) {
    return std::nullopt;
  }
  return offset_of(value_ptr);
}

template <Derived<dom::Value> T>
bool MemoryManager::write(Offset offset, T &&value) {
  if (!is_valid(offset)) {
    return false;
  }
  T *data = reinterpret_cast<T *>(address_of(offset));
  *data   = value;
  return true;
}

template <typename T>
Offset MemoryManager::alloc(size_t size) {
  size_t alloc_size     = size * sizeof(T) + sizeof(size);
  size_t arena_size_idx = fit_arena_idx(alloc_size);
  while (arena_size_idx < mem::kNumAvailableSizes &&
         is_null(mem_view_.header->free_fixed_arena[arena_size_idx])) {
    ++arena_size_idx;
  }
  if (arena_size_idx < mem::kNumAvailableSizes) {
    return use_arena(mem_view_.header->free_fixed_arena[arena_size_idx]);
  }
  if (arena_size_idx == mem::kNumAvailableSizes &&
      !is_null(mem_view_.header->free_ext_arena)) {
    // TODO: split extended arena into smaller
    return use_arena(mem_view_.header->free_ext_arena);
  }
  constexpr size_t extra_size = sizeof(Arena::size);

  std::byte *empty  = expand_file_by(alloc_size + extra_size);
  auto empty_arena  = reinterpret_cast<Arena *>(empty);
  empty_arena->size = alloc_size;

  mem_view_.header->last_arena = Offset{data_offset_in(empty_arena)};

  auto count = reinterpret_cast<size_t *>(empty_arena->data.bytes);
  *count     = size;

  auto free_block = Offset{offset_of(count + 1)};
  return free_block;
}

} // namespace mem
