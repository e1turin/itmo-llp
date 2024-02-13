#pragma once
#include <store/memory/memory_manager.h>

class Storage {
public:
  Storage(mem::MemoryManager, std::string_view);
  void Create();
  void Read();
  void Update();
  void Delete();
private:
  mem::MemoryManager memm_;
};