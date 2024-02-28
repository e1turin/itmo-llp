#pragma once

#include "file_layout.h"
#include "offset.h"
#include "store/dom/value.h"
#include "store/fs/file.h"
#include "util/util.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>

namespace mem {

class MemoryManager final {
public:

  explicit MemoryManager(std::unique_ptr<fs::File> file, bool init = false);
  ~MemoryManager();

  /**
   * Returns offset of rool element in file.
   * @return Optional result with offset to root element in file.
   */
  [[nodiscard]]
  std::optional<mem::Offset> root_ref() const;

  /**
   * Returns data of object of specified type stored in block with given offset.
   * @tparam T Type of stored object.
   * @param offset Offset to block to read from.
   * @return Optional result with required object data.
   */
  template <typename T>
  [[nodiscard]]
  std::optional<T> read(Offset offset) const;

  template <typename T>
  [[nodiscard]]
  std::optional<std::vector<T>> read_all(Offset) const;

  /**
   * Writes data from specified by pointer source to specified by offset block
   * with specified size.
   * @param dest Offset of destination block.
   * @param src  Pointer to source data in process memory.
   * @param size Size of bytes to be written.
   * @return true if OK
   */
  bool write(Offset dest, const void *src, size_t size) const;
  /**
   * Writes data of given object to specified memory block.
   * @tparam T Type of value to be written.
   * @param dest  Offset of destination block.
   * @param value Value to be written in specified block.
   * @return true if OK
   */
  template <typename T>
  bool write(Offset, T &&) const;
  /**
   * Copies memory from specified source block to specified destination block.
   * It's not clear memory from source so it's user headache.
   * @tparam T  Type of elements to be coppied.
   * @param dest Offset of destination block.
   * @param src  Offset of source block.
   * @param size Amount of elements of type T
   * @return true if OK
   */
  template <typename T>
  [[nodiscard]] bool move(Offset dest, Offset src, size_t size) const;

  /**
   * Allocates memory block for specified amount of elements of specified size.
   * @tparam T Type of elements to be stored in block.
   * @param size Amount of elements of specified type needs to allocate.
   * @param occupy Flag to specify whether allocated arena needs to be
   * initialized with size or not.
   * @return Not zero offset to freed space. If occupy flag is set to true (default) than
   * in the beginnig of block is saves size else 0.
   */
  template <typename T>
  [[nodiscard]]
  Offset alloc(size_t size, bool occupy = true);
  /**
   * Tries to expand block of memory allocated previously. If it can't it will
   * allocate new one and return offset of it.
   * @tparam T Type of elements to be stored in the block.
   * @param initial Offset to current block of memory.
   * @param new_size Required new size of block.
   * @return Offset to block of memory of required size (previous or).
   */
  template <typename T>
  [[nodiscard]] Offset realloc(Offset initial, size_t new_size);
  /**
   * Frees allocated previously block of memory.
   * @return Size of bytes been freed.
   */
  size_t free(Offset);

  template <typename T>
  [[nodiscard]] std::optional<std::vector<std::pair<Offset, Offset>>>
      ref_all_pairs(Offset) const;
  template <typename T, typename R>
  std::optional<R> do_for_entries(Offset offset,
                                  std::function<R(size_t, T *)> func);
  template <typename T, typename R>
  std::optional<Offset> find_in_entries(Offset,
                                        std::function<R *(size_t, T *)>);
  template <typename T, typename R>
  std::optional<std::vector<Offset>>
  find_in_entries(Offset, std::function<std::vector<R *>(size_t, T *)>);

private:
  friend class MemoryManagerTest;

  [[nodiscard]] bool check_header() const;
  void setup_header() const;
  [[nodiscard]] std::byte *expand_file_by(size_t);
  void map_file();
  void unmap_file();
  void remap_file();
  [[nodiscard]] Arena *arena_for(Offset) const;
  [[nodiscard]] Offset data_offset_in(Arena *) const;
  [[nodiscard]] Offset use_arena(Offset) const;
  [[nodiscard]] std::byte *address_of(Offset) const;
  [[nodiscard]] Offset offset_of(void *) const;
  [[nodiscard]] bool is_valid(Offset) const;
  [[nodiscard]] static bool is_null(Offset);

  std::unique_ptr<fs::File> file_;
  size_t file_size_;
  HANDLE file_map_obj_;
  MemView mem_view_;
};

template <typename T>
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
  auto data = reinterpret_cast<std::pair<T, T> *>(size + 1);
  std::vector<std::pair<Offset, Offset>> pairs;
  pairs.reserve(*size);
  for (size_t i = 0; i < *size; ++i) {
    pairs.emplace_back(offset_of(&data[i].first), offset_of(&data[i].second));
  }
  return pairs;
}

template <typename T, typename R>
std::optional<R>
MemoryManager::do_for_entries(Offset offset,
                              std::function<R(size_t, T *)> func) {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  auto data = reinterpret_cast<T *>(size + 1);
  return std::optional{func(*size, data)};
}

template <typename T, typename R>
std::optional<Offset>
MemoryManager::find_in_entries(Offset offset,
                               std::function<R *(size_t, T *)> func) {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  auto data = reinterpret_cast<T *>(size + 1);

  auto value_ptr = func(*size, data);
  if (value_ptr == nullptr) { // not found
    return Offset{0};
  }
  Offset target = offset_of(value_ptr);
  return is_valid(target) ? std::make_optional(target) : std::nullopt;
}

template <typename T, typename R>
std::optional<std::vector<Offset>> MemoryManager::find_in_entries(
    Offset offset, std::function<std::vector<R *>(size_t, T *)> func) {
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  auto size = reinterpret_cast<size_t *>(address_of(offset));
  auto data = reinterpret_cast<T *>(size + 1);

  std::vector<R *> ptrs = func(*size, data);
  for (auto ptr : ptrs) {
    if (!is_valid(Offset{offset_of(ptr)})) {
      return std::nullopt;
    }
  }
  std::vector<Offset> refs;
  std::transform(ptrs.begin(), ptrs.end(), std::back_inserter(refs),
                 [&](R *ptr) -> Offset { return Offset{offset_of(ptr)}; });
  return refs;
}

template <typename T>
bool MemoryManager::write(Offset dest, T &&value) const {
  if (!is_valid(dest)) {
    return false;
  }
  T *data = reinterpret_cast<T *>(address_of(dest));
  *data   = value;
  return true;
}

template <typename T>
bool MemoryManager::move(Offset dest, Offset src, size_t size) const {
  if (!is_valid(dest) || !is_valid(src) ||
      arena_for(dest)->size < size || arena_for(src)->size < size) {
    return false;
  }
  if (dest.value() == src.value()) {
    return true;
  }
  size_t move_size = size * sizeof(T) + sizeof(size);
  memmove(address_of(dest), address_of(src), move_size);
  return true;
}

template <typename T>
Offset MemoryManager::alloc(size_t size, bool occupy) {
  // TODO: GRANULARITY OF MEMORY ALLOCATIONS
  size_t alloc_size_init =
      max(size * sizeof(T) + sizeof(size), mem::kMinArenaSizeInBytes);
  size_t arena_size_idx = fit_arena_idx(alloc_size_init);
  if (arena_size_idx < mem::kNumAvailableSizes &&
      !is_null(mem_view_.header->free_fixed_arena[arena_size_idx])) {
    return use_arena(mem_view_.header->free_fixed_arena[arena_size_idx]);
  }
  if (arena_size_idx == mem::kNumAvailableSizes &&
      !is_null(mem_view_.header->free_ext_arena)) {
    return use_arena(mem_view_.header->free_ext_arena);
  }
  size_t alloc_size     = arena_size_idx < mem::kNumAvailableSizes
                          ? mem_view_.header->sizes.values[arena_size_idx]
                          : alloc_size_init;

  constexpr size_t extra_size = sizeof(Arena::size);

  std::byte *empty  = expand_file_by(alloc_size + extra_size);
  auto empty_arena  = reinterpret_cast<Arena *>(empty);
  empty_arena->size = alloc_size;

  mem_view_.header->last_arena = Offset{data_offset_in(empty_arena)};

  auto count = reinterpret_cast<size_t *>(empty_arena->data.bytes);
  *count     = occupy ? size : 0;

  Offset offset = offset_of(count);
  if (!is_valid(offset)) {
    throw std::runtime_error{"Bad allocation: zero offset to new arena."};
  }
  return Offset{offset};
}

template <typename T>
Offset MemoryManager::realloc(Offset initial, size_t new_size) {
  size_t alloc_size = new_size * sizeof(T) + sizeof(new_size);
  Arena *curr_arena = arena_for(initial);
  if (curr_arena->size >= alloc_size) {
    return initial;
  }
  return alloc<T>(new_size);
}

} // namespace mem
