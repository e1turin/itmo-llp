#include "memory_manager.h"
#include "file_layout.h"
#include "store/fs/file.h"

#include <format>
#include <iostream>

namespace mem {

MemoryManager::MemoryManager(fs::File &&file) : file_(file) {
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_.handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  if (liFileSize.QuadPart == 0ll) {
    liFileSize.QuadPart = 1 << 12; // 4 KB
    if (!SetFilePointerEx(file_.handle(), liFileSize, nullptr, FILE_BEGIN)) {
      throw std::runtime_error{"Can't resize file. It still has zero size."};
    }
  }
  map_file();
}

std::optional<dom::Value> MemoryManager::get_root() const {
  return mem_view_.header->root;
}

bool MemoryManager::write(const Offset dest, const void *src,
                          const size_t size) {
  if (!is_valid(dest)) {
    return false;
  }
  void *data  = mem_view_.data + dest.value();
  void *res   = memcpy(data, src, size);
  return res != nullptr;
}


size_t MemoryManager::free(const Offset offset) const {
  Arena *arena = arena_from(offset);
  size_t arena_size_idx = fit_arena_idx(arena->size);
  if (arena_size_idx < mem::kNumAvailableSizes) {
    // TODO: arenas can be sorted by offset
    arena->data.next = mem_view_.header->free_fixed_arena[arena_size_idx];
    mem_view_.header->free_fixed_arena[arena_size_idx] = offset;
  } else {
    arena->data.next = mem_view_.header->free_ext_arena;
    mem_view_.header->free_ext_arena = offset;
  }
}

std::byte *MemoryManager::address_of(const Offset offset) const {
  return mem_view_.data + offset.value();
}

bool MemoryManager::is_valid(Offset offset) const {
  if (offset.value() == 0) {
    return false;
  } else {
    return true;
  }
}
bool MemoryManager::is_null(Offset offset) { return offset.value() == 0; }

std::byte *MemoryManager::extend_file(size_t size) {
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_.handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  LARGE_INTEGER liNewFileSize;
  liNewFileSize.QuadPart = liFileSize.QuadPart + size;

  if (!SetFilePointerEx(file_.handle(), liNewFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{std::format("Can't resize file for {} by {}",
                                         liFileSize.QuadPart, size)};
  }

  map_file();

  return mem_view_.data + liFileSize.QuadPart;
}

void MemoryManager::map_file() {
  constexpr DWORD dwMaximumSizeHigh = 0; // To map size equal to
  constexpr DWORD dwMaximumSizeLow  = 0; // the whole file size.

  file_map_obj_ =
      CreateFileMapping(file_.handle(), nullptr, PAGE_READWRITE,
                        dwMaximumSizeHigh, dwMaximumSizeLow, nullptr);

  if (file_map_obj_ == nullptr) {
    throw std::runtime_error{"Can't map file."};
  }

  constexpr DWORD dwFileOffsetHigh     = 0;
  constexpr DWORD dwFileOffsetLow      = 0;
  constexpr DWORD dwNumberOfBytesToMap = 0; // Till the end of file.

  mem_view_.view_ptr =
      MapViewOfFile(file_map_obj_, FILE_MAP_ALL_ACCESS, dwFileOffsetHigh,
                    dwFileOffsetLow, dwNumberOfBytesToMap);
  if (mem_view_.view_ptr == nullptr) {
    throw std::runtime_error{"Can't create mapping view"};
  }
}

Arena *MemoryManager::arena_from(Offset offset) const {
  std::ptrdiff_t diff = offsetof(Arena, data);
  auto arena =
      reinterpret_cast<Arena *>(mem_view_.data + offset.value() - diff);
  return arena;
}

Offset MemoryManager::use_arena(Offset offset) const {
  Arena *arena     = arena_from(offset);
  size_t arena_idx = fit_arena_idx(arena->size);
  if (arena_idx < mem::kNumAvailableSizes) {
    mem_view_.header->free_fixed_arena[arena_idx] =
        is_valid(arena->data.next) ? arena->data.next : Offset{0};
  } else {
    mem_view_.header->free_ext_arena =
        is_valid(arena->data.next) ? arena->data.next : Offset{0};
  }
  return offset;
}

Offset MemoryManager::data_offset_from(Arena *arena) const {
  std::byte *arena_data = arena->data.bytes;
  size_t offset         = arena_data - mem_view_.data;
  return Offset{offset};
}
Offset MemoryManager::offset_of(void *p) const {
  auto *data    = static_cast<std::byte *>(p);
  size_t offset = data - mem_view_.data;
  return Offset{offset};
}

} // namespace mem
