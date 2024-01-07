#ifndef ITMO_LLP_STORE_MEMORY_ARENA_ALLOC_H_
#define ITMO_LLP_STORE_MEMORY_ARENA_ALLOC_H_

#include <cstdint>

namespace store::memory {

class ArenaAlloc {
 public:
  ArenaAlloc(uint8_t* block, size_t size);
 private:
};

}

#endif  // ITMO_LLP_STORE_MEMORY_ARENA_ALLOC_H_
