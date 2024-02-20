#include "memory_manager.h"
#include "file_layout.h"
#include "store/fs/file.h"

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

  constexpr DWORD dwMaximumSizeHigh = 0; // To map size equal to
  constexpr DWORD dwMaximumSizeLow  = 0; // the whole file size.

  HANDLE file_map_obj_ =
      CreateFileMapping(file_.handle(), nullptr, PAGE_READWRITE,
                        dwMaximumSizeHigh, dwMaximumSizeLow, nullptr);

  if (file_map_obj_ == nullptr) {
    throw std::runtime_error{"Can't map file."};
  }

  alloc_ =
      std::make_unique<mem::ArenaAlloc>(file_map_obj_, liFileSize.QuadPart);
}

std::optional<dom::Value> MemoryManager::get_root() const {
  return alloc_->get_root();
}

bool MemoryManager::write(const Offset dest, const std::byte *begin,
                          const std::byte *end) {
  if (begin > end || !is_valid(dest)) {
    return false;
  }
  void *data  = alloc_->data_ptr() + dest.value();
  size_t size = end - begin;
  void *res   = memcpy(data, begin, size);
  return res != nullptr;
}

Offset MemoryManager::alloc(const size_t size) const {
  return alloc_->alloc(size);
}

size_t MemoryManager::free(const Offset offset) const {
  return alloc_->free(offset);
}

std::byte *MemoryManager::address_of(const Offset offset) const {
  return alloc_->data_ptr() + offset.value();
}

bool MemoryManager::is_valid(Offset offset) const {
  if (offset.value() == 0) {
    return false;
  } else {
    return true;
  }
}

} // namespace mem
