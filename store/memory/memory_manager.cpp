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
}

MemoryManager::~MemoryManager() {
  if (!UnmapViewOfFile(file_view_begin_)) {
    std::cerr << "ERROR: Can't unmap file" << std::endl;
  }

  CloseHandle(file_map_obj_);
}

/* MemoryManager::read overloads */

std::int32_t MemoryManager::read(const dom::Int32Value &v) {
  return v.get_int();
}
float MemoryManager::read(const dom::Float32Value &v) { return v.get_float(); }
bool MemoryManager::read(const dom::BoolValue &v) { return v.get_bool(); }

// todo: explicit type check
std::optional<std::string_view>
MemoryManager::read(const dom::StringValue &str) const {
  fs::Offset offset = str.get_ref();
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  std::byte *data = address_of(offset);

  auto size  = reinterpret_cast<size_t *>(data);
  auto begin = reinterpret_cast<char *>(size + 1);
  return std::move(std::string{begin, *size});
}

std::optional<char> MemoryManager::read(const dom::StringValue &str,
                                        size_t idx) const {
  fs::Offset offset = str.get_ref();
  if (!is_valid(offset)) {
    return std::nullopt;
  }
  std::byte *data = address_of(offset);

  auto size = reinterpret_cast<size_t *>(data);
  if (idx >= *size) {
    return std::nullopt;
  }
  auto begin = reinterpret_cast<char *>(size + 1);
  return *(begin + idx);
}

std::unique_ptr<std::vector<dom::Entry>>
MemoryManager::read(const dom::ObjectValue &obj) const {
  fs::Offset offset = obj.get_ref();
  if (!is_valid(offset)) {
    return nullptr;
  }
  std::byte *data = address_of(offset);

  auto size  = reinterpret_cast<size_t *>(data);
  auto begin = reinterpret_cast<dom::Entry *>(size + 1);
  auto end   = begin + *size;
  return std::make_unique<std::vector<dom::Entry>>(begin, end);
}

std::unique_ptr<dom::Entry> MemoryManager::read(const dom::ObjectValue &obj,
                                                size_t idx) const {
  fs::Offset offset = obj.get_ref();
  if (!is_valid(offset)) {
    return nullptr;
  }
  std::byte *data = address_of(offset);

  auto size = reinterpret_cast<size_t *>(data);
  if (idx >= *size) {
    return nullptr;
  }
  auto begin = reinterpret_cast<dom::Entry *>(size + 1);
  return std::make_unique<dom::Entry>(*(begin + idx));
}

std::unique_ptr<dom::Value> MemoryManager::read(const dom::ObjectValue &obj,
                                                std::string_view key) const {
  fs::Offset offset = obj.get_ref();
  if (!is_valid(offset)) {
    return nullptr;
  }
  std::byte *data = address_of(offset);

  auto size       = reinterpret_cast<size_t *>(data);
  auto begin      = reinterpret_cast<dom::Entry *>(size + 1);
  dom::Entry *end = begin + *size;

  auto val = std::find_if(begin, end, [&](dom::Entry it) -> bool {
    std::optional<std::string_view> str = read(it.key);
    if (str->empty()) {
      return false;
    } else {
      return str.value() == key;
    }
  });
  return std::make_unique<dom::Value>(val->value);
}

/* MemoryManager::write overloads */

bool MemoryManager::write(dom::ObjectValue &obj, std::string_view key,
                          std::int32_t val) {}
bool MemoryManager::write(dom::ObjectValue &obj, std::string_view key,
                          float val) {}
bool MemoryManager::write(dom::ObjectValue &obj, std::string_view key,
                          bool val) {}

fs::Offset MemoryManager::alloc(const size_t size) const {
  // TODO: check free list
  // TODO: remap file

  DWORD dwFileSizeHigh;
  const DWORD dwFileSizeLow = GetFileSize(file_.handle(), &dwFileSizeHigh);

  const size_t file_size = (dwFileSizeHigh << 32) + dwFileSizeLow;

  if (dwFileSizeLow == INVALID_FILE_SIZE) {
    throw std::runtime_error{"Can't get file size"};
  }

  // TODO: handle size > unsigned long
  const size_t new_file_size = file_size + size;
  const LONG lNewFileSize    = static_cast<LONG>(new_file_size >> 32);
  // const PLONG lNewFileSizeHigh = reinterpret_cast<const long
  // *>(&new_file_size) + 1;

  //TODO use ..Ex alternative
  if (SetFilePointer(file_.handle(), lNewFileSize, nullptr, FILE_BEGIN)) {
    throw std::runtime_error{"Can't resize file. It has zero size."};
  }

  return fs::Offset{file_size};
}

size_t MemoryManager::free(const fs::Offset offset, const size_t size) const {
  //TODO good freeing
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
