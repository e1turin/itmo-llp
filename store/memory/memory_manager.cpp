#include "memory_manager.h"
#include "store/fs/file.h"

#include <iostream>
#include <format>

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

  file_map_obj_ =
      CreateFileMapping(file_.handle(), nullptr, PAGE_READWRITE,
                        dwMaximumSizeHigh, dwMaximumSizeLow, nullptr);

  if (file_map_obj_ == nullptr) {
    throw std::runtime_error{"Can't map file."};
  }

  constexpr DWORD dwFileOffsetHigh     = 0;
  constexpr DWORD dwFileOffsetLow      = 0;
  constexpr DWORD dwNumberOfBytesToMap = 0; // Till the end of file.

  file_view_.view_ptr = static_cast<void *>(
      MapViewOfFile(file_map_obj_, FILE_MAP_ALL_ACCESS, dwFileOffsetHigh,
                    dwFileOffsetLow, dwNumberOfBytesToMap));

  alloc_ = std::make_unique<mem::ArenaAlloc>();
}

MemoryManager::~MemoryManager() {
  if (!UnmapViewOfFile(file_view_.view_ptr)) {
    std::cerr << "ERROR: Can't unmap file" << std::endl;
  }

  CloseHandle(file_map_obj_);
}

/* MemoryManager::write overloads */

fs::Offset MemoryManager::alloc(const size_t size) const {
  LARGE_INTEGER liFileSize;
  if (!GetFileSizeEx(file_.handle(), &liFileSize)) {
    throw std::runtime_error{"Can't get file size."};
  }

  liFileSize.QuadPart += size;

  if (!SetFilePointerEx(file_.handle(), liFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{std::format("Can't resize file for {} by {}", liFileSize.QuadPart, size)};
  }

  return fs::Offset{static_cast<size_t>(liFileSize.QuadPart)};
}

size_t MemoryManager::free(const fs::Offset offset, const size_t size) const {
  // TODO good freeing
  memset(address_of(offset), 0, size);
  return size;
}

std::byte *MemoryManager::address_of(const fs::Offset offset) const {
  return file_view_.data + offset.value();
}

bool MemoryManager::is_valid(fs::Offset offset) const {
  if (offset.value() == 0) {
    return false;
  } else {
    return true;
  }
}

} // namespace mem
