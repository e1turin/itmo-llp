#include "arena.h"
#include "file_layout.h"

#include <iostream>
#include <stdexcept>

namespace mem {

ArenaAlloc::ArenaAlloc(HANDLE file_map_obj, size_t size)
    : file_map_obj_(file_map_obj), size_(size) {
  constexpr DWORD dwFileOffsetHigh     = 0;
  constexpr DWORD dwFileOffsetLow      = 0;
  constexpr DWORD dwNumberOfBytesToMap = 0; // Till the end of file.

  mem_view_.view_ptr =
      MapViewOfFile(file_map_obj_, FILE_MAP_ALL_ACCESS, dwFileOffsetHigh,
                    dwFileOffsetLow, dwNumberOfBytesToMap);
  if(mem_view_.view_ptr == nullptr) {
    throw std::runtime_error{"Can't create mapping view"};
  }
}

ArenaAlloc::~ArenaAlloc() {
  if (!UnmapViewOfFile(mem_view_.view_ptr)) {
    std::cerr << "ERROR: Can't unmap file" << std::endl;
  }
  CloseHandle(file_map_obj_);
}

dom::Value ArenaAlloc::get_root() const {
  return mem_view_.header->root;
}
std::byte *ArenaAlloc::data_ptr() const { return mem_view_.data; }

Offset ArenaAlloc::alloc(size_t size) {
/*
  size_t arena_size_idx = fit_arena_idx(size);
  Offset offset{0};
  if (arena_size_idx < kNumAvailableSizes) {
    offset = free_fixed_arena[arena_size_idx];
    while (is_null(offset) && arena_size_idx < kNumAvailableSizes) {
      // TODO: split in 2 smaller arenas
      offset = meta_info_->free_fixed_arena[++arena_size_idx];
    }
    if (is_null(offset)) {
      offset = meta_info_->free_ext_arena;
    }
  } else {
    offset = meta_info_->free_ext_arena;
  }
  if (!is_valid(offset)){
    lack = size;
    return nullptr;
  }
  auto arena = reinterpret_cast<Arena *>(data_ + offset.value());
  return arena;
*/
}

bool ArenaAlloc::is_valid(Offset offset) {
  //TODO: consider alignment & referred data content tag
  if (offset.value() == 0){
    return false;
  }
  return true;
}

bool ArenaAlloc::is_null(Offset offset) { return offset.value() == 0; }

bool ArenaAlloc::map_memory() {
/*
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_.handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }

  liFileSize.QuadPart += size;

  if (!SetFilePointerEx(file_.handle(), liFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{std::format("Can't resize file for {} by {}", liFileSize.QuadPart, size)};
  }

  return Offset{static_cast<size_t>(liFileSize.QuadPart)};
*/
}

Arena *ArenaAlloc::arena_from(Offset offset) {
  std::ptrdiff_t diff = offsetof(Arena, data);
  auto arena = reinterpret_cast<Arena *>(data_ptr() + offset.value() - diff);
  return arena;
}

} // namespace mem
