#include "storage.h"

#include <format>

Storage::Storage(std::string_view file_name) {
  auto db_file = fs::File(file_name);

  memm_ = std::make_unique<mem::MemoryManager>(std::move(db_file));
}
