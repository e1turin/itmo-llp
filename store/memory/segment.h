#pragma once
#include <memory>
#include <store/fs/filesystem.h>
#include <utility>

namespace mem {

struct Segment {
  friend class SegmentAlloc;

  const std::shared_ptr<fs::File> file;
  const fs::File::Mode mode;
  const size_t offset;
  const size_t size;
  const uint8_t *begin;
  const uint8_t *meta_info;

private:
  Segment(const std::shared_ptr<fs::File>& _file, const fs::File::Mode _mode,
          const size_t _offset, const size_t _size, const uint8_t *_begin,
          const uint8_t *_meta_info)
      : file(_file), mode(_mode), offset(_offset), size(_size), begin(_begin),
        meta_info(_meta_info) {}
};

class SegmentAlloc {
public:
  [[nodiscard]] std::unique_ptr<Segment> alloc(size_t size);

  [[nodiscard]] std::unique_ptr<Segment> alloc(fs::File::Offset, size_t size,
                                               fs::File::Mode);

  size_t free(std::unique_ptr<Segment>);
};

} // namespace mem
