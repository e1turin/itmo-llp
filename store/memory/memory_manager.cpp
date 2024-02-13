#include "memory_manager.h"
#include "store/fs/filesystem.h"

#include <iostream>

namespace mem {

MemoryManager::MemoryManager(std::unique_ptr<fs::FileManager> fm,
                             std::string_view filename)
    : file_manager_(std::move(fm)) {
  file_ = file_manager_->open(filename);
  if (file_ == nullptr) {
    throw std::runtime_error("Can't open DB file");
  }

  DWORD dwFileSize = GetFileSize(file_->handle(), nullptr);

  if (dwFileSize == INVALID_FILE_SIZE) {
    throw std::runtime_error("Can't get file size");
  }

  if (dwFileSize == 0) {
    LONG lNewFileSize = 1024;
    if (SetFilePointer(file_->handle(), lNewFileSize, nullptr, FILE_BEGIN)) {
      throw std::runtime_error("Can't resize file. It has zero size.");
    }
  }

  DWORD dwMaximumSizeHigh = 0; // whole file size
  DWORD dwMaximumSizeLow  = 0;

  file_map_obj_ =
      CreateFileMapping(file_->handle(), nullptr, PAGE_READWRITE,
                        dwMaximumSizeHigh, dwMaximumSizeLow, nullptr);

  if (file_map_obj_ == nullptr) {
    std::cerr << "ERROR: can't map file into memory; " << GetLastError()
              << std::endl;
    throw std::runtime_error("Can't map file");
  }

  DWORD dwFileOffsetHigh     = 0;
  DWORD dwFileOffsetLow      = 0;
  DWORD dwNumberOfBytesToMap = 0; // till the end of file

  file_view_begin_ =
      MapViewOfFile(file_map_obj_, FILE_MAP_ALL_ACCESS, dwFileOffsetHigh,
                    dwFileOffsetLow, dwNumberOfBytesToMap);
}

MemoryManager::~MemoryManager() {
  if (!UnmapViewOfFile(file_view_begin_)) {
    std::cerr << "ERROR: Can't unmap file" << std::endl;
  }

  CloseHandle(file_map_obj_);
  file_manager_->close(file_);
}

fs::Offset MemoryManager::alloc(size_t size) {
  const DWORD dwFileSize = GetFileSize(file_->handle(), nullptr);

  if (dwFileSize == INVALID_FILE_SIZE) {
    throw std::runtime_error("Can't get file size");
  }

  // TODO: handle size > unsigned long
  const LONG lNewFileSize = dwFileSize + size;

  if (SetFilePointer(file_->handle(), lNewFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error("Can't resize file. It has zero size.");
  }

  return fs::Offset{lNewFileSize};
}

size_t MemoryManager::free(fs::Offset, size_t) {}

} // namespace mem
