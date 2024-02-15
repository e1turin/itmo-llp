#include "file.h"

#include <format>
#include <iostream>

namespace fs {

File::File(std::string_view name) : name_(name) {
  DWORD dwAccessMode          = GENERIC_READ | GENERIC_WRITE;
  constexpr DWORD dwShareMode = 0; // exclusive

  HANDLE hFile = CreateFile(name.data(), dwAccessMode, dwShareMode, nullptr,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE) {
    throw std::runtime_error{std::format(R"(Can't open file "{}")", name)};
  }
  handle_ = hFile;
}

File::~File() {
  if (!CloseHandle(handle())) {
    std::cerr << "Can't close file " << name_ << std::endl;
  }
}

} // namespace fs
