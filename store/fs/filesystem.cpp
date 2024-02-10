#include "filesystem.h"
#include <fcntl.h>

namespace fs {

File::File(const int fd) : handle_(fd) {}

std::unique_ptr<File> FileManager::open(const std::string_view filename) {
  int fd = ::open(filename.data(), O_RDONLY);
  if (fd == -1)
    return nullptr;

  return std::make_unique<File>{fd};
}

 File::~File() {
  close(handle_);
}


} // namespace fs
