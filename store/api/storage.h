#pragma once
#include <optional>
#include <store/dom/value.h>
#include <store/memory/memory_manager.h>
#include <util/util.h>

class Storage final {
public:
  explicit Storage(std::string_view);

  [[nodiscard]] // todo: const Value
  std::optional<dom::Value> root() const;

  /* todo: std::variant for result with error code */
  [[nodiscard]]
  static std::optional<std::int32_t> read(const dom::Int32Value &) ;
  [[nodiscard]]
  static std::optional<float> read(const dom::Float32Value &) ;
  [[nodiscard]]
  static std::optional<bool> read(const dom::BoolValue &) ;
  [[nodiscard]]
  std::optional<std::string_view> read(const dom::StringValue &) const;
  [[nodiscard]]
  std::optional<std::vector<dom::Entry>> read(const dom::ObjectValue &) const;

  template <typename T, dom::Value::Type vtype>
  [[nodiscard]]
  std::optional<T> get(const dom::VectorValue<T, vtype> &, size_t) const;
  [[nodiscard]]
  std::optional<dom::Value> get(const dom::ObjectValue &,
                                std::string_view) const;

  /* todo set value by index for string and mb obj */
  bool set(dom::ObjectValue &, std::string_view, bool) const;
  bool set(dom::ObjectValue &, std::string_view, std::int32_t) const;
  bool set(dom::ObjectValue &, std::string_view, float) const;
  size_t set(dom::ObjectValue &, std::string_view, std::string_view) const;
  std::optional<dom::ObjectValue> set(dom::ObjectValue &, std::string_view,
                                        size_t) const;

  bool truncate(dom::ObjectValue &, size_t) const;
  bool truncate(dom::ObjectValue &, std::string_view) const;

private:
  std::unique_ptr<mem::MemoryManager> memm_;
};


template <typename T, dom::Value::Type vtype>
std::optional<T> Storage::get(const dom::VectorValue<T, vtype> &vec,
                              size_t i) const {
  auto vals = read(vec);
  if(!vals.has_value() || i >= vals->size()) {
    return std::nullopt;
  }
  return vals[i];
}

