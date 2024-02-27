#include "memory_manager.h"
#include "file_layout.h"
#include "store/fs/file.h"

#include <format>
#include <iostream>

namespace mem {

MemoryManager::MemoryManager(std::unique_ptr<fs::File> file, bool init)
    : file_(std::move(file)) {
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_->handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  if (liFileSize.QuadPart == 0ll) {
    liFileSize.QuadPart = 1 << 12; // 4 KB
    if (!SetFilePointerEx(file_->handle(), liFileSize, nullptr, FILE_BEGIN)) {
      throw std::runtime_error{"Can't resize file. It still has zero size."};
    }
    if (!SetEndOfFile(file_->handle())) {
      throw std::runtime_error{"Can't save file size. It will have zero size."};
    }
  }
  file_size_ = liFileSize.QuadPart;
  map_file();
  if (!check_header()) {
    if (init) {
      setup_header();
    } else {
      throw std::runtime_error{"Unspecified file format. Use additional flag to reset."};
    }
  };
}

MemoryManager::~MemoryManager() {
  unmap_file();
}

std::optional<mem::Offset> MemoryManager::root_ref() const {
  return offset_of(&mem_view_.header->root);
}

bool MemoryManager::write(const Offset dest, const void *src,
                          const size_t size) const {
  if (!is_valid(dest)) {
    return false;
  }
  void *data  = mem_view_.data + dest.value();
  void *res   = memcpy(data, src, size);
  return res != nullptr;
}

size_t MemoryManager::free(const Offset offset) const {
  if (!is_valid(offset)) {
    return 0;
  }
  Arena *arena = arena_for(offset);
  size_t arena_size_idx = fit_arena_idx(arena->size);
  if (arena_size_idx < mem::kNumAvailableSizes) {
    // todo: arenas can be sorted by offset
    arena->data.next = mem_view_.header->free_fixed_arena[arena_size_idx];
    mem_view_.header->free_fixed_arena[arena_size_idx] = offset;
  } else {
    arena->data.next = mem_view_.header->free_ext_arena;
    mem_view_.header->free_ext_arena = offset;
  }
  return arena->size;
}

std::byte *MemoryManager::address_of(const Offset offset) const {
  return mem_view_.data + offset.value();
}
Arena *MemoryManager::arena_for(Offset offset) const {
  std::ptrdiff_t diff = offsetof(Arena, data);
  auto arena =
      reinterpret_cast<Arena *>(mem_view_.data + offset.value() - diff);
  return arena;
}
Offset MemoryManager::data_offset_in(Arena *arena) const {
  std::byte *arena_data = arena->data.bytes;
  size_t offset         = arena_data - mem_view_.data;
  return Offset{offset};
}
Offset MemoryManager::offset_of(void *p) const {
  auto *data    = static_cast<std::byte *>(p);
  size_t offset = data - mem_view_.data;
  auto off = Offset{offset};
  return is_valid(off) ? off : Offset{0};
}

bool MemoryManager::is_null(Offset offset) { return offset.value() == 0; }
bool MemoryManager::is_valid(Offset offset) const {
  return offset.value() == offsetof(FileHeader, root) ||
         offset.value() >= sizeof(FileHeader) && offset.value() < file_size_;
}

void MemoryManager::map_file() {
  constexpr DWORD dwMaximumSizeHigh = 0; // To map size equal to
  constexpr DWORD dwMaximumSizeLow  = 0; // the whole file size.

  file_map_obj_ =
      CreateFileMapping(file_->handle(), nullptr, PAGE_READWRITE,
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

void MemoryManager::unmap_file() {
    if(!CloseHandle(file_map_obj_)) {
      std::cerr << "Can't delete mapping object" << std::endl;
    }
    mem_view_.view_ptr = nullptr;
}

void MemoryManager::remap_file() {
  unmap_file();
  map_file();
}

std::byte *MemoryManager::expand_file_by(size_t size) {
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_->handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }
  LARGE_INTEGER liNewFileSize;
  liNewFileSize.QuadPart = liFileSize.QuadPart + size;

  if (!SetFilePointerEx(file_->handle(), liNewFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{std::format("Can't resize file for {} by {}",
                                         liFileSize.QuadPart, size)};
  }
  if (!SetEndOfFile(file_->handle())) {
    throw std::runtime_error{"Can't save file size for new expansion."};
  }
  remap_file();
  file_size_ = liNewFileSize.QuadPart;
  memset(mem_view_.data + liFileSize.QuadPart, 0, size);
  return mem_view_.data + liFileSize.QuadPart;
}

Offset MemoryManager::use_arena(Offset offset) const {
  Arena *arena     = arena_for(offset);
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
bool MemoryManager::check_header() const {
  return mem_view_.header->magic == mem::kMagic;
}
void MemoryManager::setup_header() const {
  constexpr size_t header_size =  sizeof(*mem_view_.header);
  memset(mem_view_.header, 0, header_size);
  mem_view_.header->magic = mem::kMagic;
  mem_view_.header->root = dom::ObjectValue::null_object().as<dom::Value>();
  mem_view_.header->sizes = ArenaSizes{};
  mem_view_.header->free_ext_arena = mem::Offset{sizeof(FileHeader)};
  Arena *free = arena_for(mem_view_.header->free_ext_arena);
  free->size = file_size_ - sizeof(FileHeader);
}

} // namespace mem
