#pragma once

#include <store/dom/value.h>
#include <store/fs/file.h>
#include <util/util.h>

namespace mem {

class MemoryManager final {
public:
  explicit MemoryManager(fs::File &&);

  ~MemoryManager();

  [[nodiscard]] std::unique_ptr<dom::Value> read(fs::Offset) const;

  static [[nodiscard]]
  std::int32_t read(const dom::Int32Value &);
  static [[nodiscard]]
  float read(const dom::Float32Value &);
  static [[nodiscard]]
  bool read(const dom::BoolValue &);

  [[nodiscard]]
  std::string_view read(const dom::StringValue &) const;
  [[nodiscard]]
  char read(const dom::StringValue &, size_t) const;
  [[nodiscard]]
  std::unique_ptr<std::vector<dom::Entry>> read(const dom::ObjectValue &) const;
  [[nodiscard]]
  std::unique_ptr<dom::Entry> read(const dom::ObjectValue &, size_t) const;
  [[nodiscard]]
  std::unique_ptr<dom::Value> read(const dom::ObjectValue &,
                                   std::string_view) const;

  bool write(dom::ObjectValue &, std::string_view, std::int32_t);
  bool write(dom::ObjectValue &, std::string_view, float);
  bool write(dom::ObjectValue &, std::string_view, bool);

  size_t write(dom::ObjectValue &, std::string_view, std::string_view);
  template <Derived<dom::ObjectValue>>
  std::unique_ptr<dom::ObjectValue> write(dom::ObjectValue &, std::string_view,
                                          size_t);

  size_t free(dom::ObjectValue &, size_t) const;
  size_t free(dom::ObjectValue &, std::string_view) const;

private:
  fs::File file_;
  HANDLE file_map_obj_;
  std::byte *file_view_begin_;

  template <typename T>
  bool write(fs::Offset, T);
  [[nodiscard]] fs::Offset alloc(size_t) const;
  [[nodiscard]] size_t free(fs::Offset, size_t) const;

  [[nodiscard]] void *address_of(fs::Offset) const;

  bool remap_file();
};

template <Derived<dom::ObjectValue>>
std::unique_ptr<dom::ObjectValue>
MemoryManager::write(dom::ObjectValue &, std::string_view, size_t size) {}

} // namespace mem
