#include "memory_manager.h"
#include "store/fs/file.h"

#include <iostream>

namespace mem {

MemoryManager::MemoryManager(fs::File &&file) : file_(file) {

  const DWORD dwFileSize = GetFileSize(file_.handle(), nullptr);

  if (dwFileSize == INVALID_FILE_SIZE) {
    throw std::runtime_error{"Can't get file size"};
  }

  if (dwFileSize == 0) {
    LONG lNewFileSize = 1024;
    if (SetFilePointer(file_.handle(), lNewFileSize, nullptr, FILE_BEGIN)) {
      throw std::runtime_error{"Can't resize file. It has zero size"};
    }
  }

  constexpr DWORD dwMaximumSizeHigh = 0; // whole file size
  constexpr DWORD dwMaximumSizeLow  = 0;

  file_map_obj_ =
      CreateFileMapping(file_.handle(), nullptr, PAGE_READWRITE,
                        dwMaximumSizeHigh, dwMaximumSizeLow, nullptr);

  if (file_map_obj_ == nullptr) {
    throw std::runtime_error{"Can't map file"};
  }

  constexpr DWORD dwFileOffsetHigh     = 0;
  constexpr DWORD dwFileOffsetLow      = 0;
  constexpr DWORD dwNumberOfBytesToMap = 0; // till the end of file

  file_view_begin_ = static_cast<std::byte *>(
      MapViewOfFile(file_map_obj_, FILE_MAP_ALL_ACCESS, dwFileOffsetHigh,
                    dwFileOffsetLow, dwNumberOfBytesToMap));
  /* TODO: NEW AREAN ALLOCATOR */
}

MemoryManager::~MemoryManager() {
  if (!UnmapViewOfFile(file_view_begin_)) {
    std::cerr << "ERROR: Can't unmap file" << std::endl;
  }

  CloseHandle(file_map_obj_);
}

/* MemoryManager::read overloads  */

/* MemoryManager::write overloads */

/* MemoryManager::write overloads */

fs::Offset MemoryManager::alloc(const size_t size) const {
  // TODO: check free list
  // TODO: remap file

  DWORD dwFileSizeHigh;
  const DWORD dwFileSizeLow = GetFileSize(file_.handle(), &dwFileSizeHigh);

  const size_t file_size = (dwFileSizeHigh << 32) + dwFileSizeLow;

  if (dwFileSizeLow == INVALID_FILE_SIZE) {
    throw std::runtime_error{"Can't get file size"};
  }

  // TODO use ..Ex alternative
  const size_t new_file_size = file_size + size;
  const LONG lNewFileSize    = static_cast<LONG>(new_file_size >> 32);
  // const PLONG lNewFileSizeHigh = reinterpret_cast<const long
  // *>(&new_file_size) + 1;

  if (SetFilePointer(file_.handle(), lNewFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{"Can't resize file. It has zero size."};
  }

  return fs::Offset{file_size};
}

size_t MemoryManager::free(const fs::Offset offset, const size_t size) const {
  // TODO good freeing
  memset(address_of(offset), 0, size);
  return size;
}

std::byte *MemoryManager::address_of(const fs::Offset offset) const {
  return file_view_begin_ + offset.value();
}

bool MemoryManager::is_valid(fs::Offset offset) const {
  if (offset.value() == 0) {
    return false;
  } else {
    return true;
  }
}

} // namespace mem
