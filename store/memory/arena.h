#pragma once

#include "segment.h"
#include <cstddef>
#include <cstdint>

namespace mem {

class Arena {
  friend class ArenaAlloc;

public:
  const uint8_t *begin() { return begin_; };
  const uint8_t *end() { return end_; };

  uint8_t get(size_t);

  void set(size_t, uint8_t);

private:
  const std::shared_ptr<Segment> segment_;
  const uint8_t *begin_;
  const uint8_t *end_;

  Arena(std::shared_ptr<Segment> &segment, const uint8_t *begin,
        const uint8_t *end)
      : segment_(segment), begin_(begin), end_(end) {}
};

class ArenaAlloc {
public:
  explicit ArenaAlloc(const std::shared_ptr<Segment> &segment)
      : segment_(segment) {}

  std::unique_ptr<Arena> alloc(size_t);

  size_t free(std::unique_ptr<Arena>);

private:
  std::shared_ptr<Segment> segment_;
  std::vector<std::unique_ptr<Arena>> free_;
};

} // namespace mem
