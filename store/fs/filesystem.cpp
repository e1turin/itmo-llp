#include "filesystem.h"
#include <iostream>

namespace fs {

const File *FileManager::open(const std::string_view &filename,
                              File::Mode mode) {
  DWORD dwAccessMode                = 0;
  const constexpr DWORD dwShareMode = 0;

  switch (mode) {
  case File::Mode::kReadWrite: dwAccessMode |= GENERIC_WRITE;
  case File::Mode::kReadOnly:  dwAccessMode |= GENERIC_READ;
  }

  const HANDLE hFile =
      CreateFile(filename.data(), dwAccessMode, dwShareMode, nullptr,
                 OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE) {
    std::cerr << "ERROR: can't open file " << filename << std::endl;
    return nullptr;
  }

  files_.emplace_back(new File{File::Handle{hFile}, mode});

  return files_.back();
}

bool FileManager::close(const File *file) {
  auto f = std::find(files_.begin(), files_.end(), file);
  if (f == files_.end()) {
    std::cerr << "ERROR: no such file under meneger control" << std::endl;
    return false;
  }

  bool bFlag = CloseHandle(file->handle().handle());

  if (bFlag) {
    files_.erase(f);
  }

  return bFlag;
}

} // namespace fs
