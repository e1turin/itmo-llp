#pragma once
#include <optional>
#include <store/dom/value.h>
#include <store/memory/memory_manager.h>
#include <util/util.h>

class Storage final {
public:
  explicit Storage(std::string_view);

  [[nodiscard]]
  std::unique_ptr<dom::Value> root() const;

  [[nodiscard]]
  std::optional<std::int32_t> read(const dom::Int32Value &) const;
  [[nodiscard]]
  std::optional<float> read(const dom::Float32Value &) const;
  [[nodiscard]]
  std::optional<bool> read(const dom::BoolValue &) const;
  [[nodiscard]]
  std::optional<std::string_view> read(const dom::StringValue &) const;
  [[nodiscard]]
  std::unique_ptr<std::vector<dom::Entry>> read(const dom::ObjectValue &) const;

  template <typename T, dom::Value::Type vtype>
  [[nodiscard]]
  std::optional<T> get(const dom::VectorValue<T, vtype> &, size_t) const;

  [[nodiscard]]
  std::unique_ptr<dom::Value> get(const dom::ObjectValue &,
                                  std::string_view) const;

  // TODO set value by index
  // template <typename T, dom::Value::Type vtype>
  // dom::Value set(dom::VectorValue<T, vtype>, size_t, T) const;

  template <typename T>
  bool set(dom::ObjectValue &, std::string_view, T) const;
  size_t set(dom::ObjectValue &, std::string_view, std::string_view) const;
  template <Derived<dom::ObjectValue>>
  std::unique_ptr<dom::ObjectValue> set(dom::ObjectValue &, std::string_view,
                                        size_t) const;

  bool trancate(dom::ObjectValue &, size_t) const;
  bool trancate(dom::ObjectValue &, std::string_view) const;

private:
  std::unique_ptr<mem::MemoryManager> memm_;
};

template <typename T, dom::Value::Type vtype>
std::optional<T> Storage::get(const dom::VectorValue<T, vtype> &v,
                              size_t i) const {
  if (v.template is<dom::StringValue>()) {
    return memm_->read(v.template as<dom::StringValue>(), i);
  }
  if (v.template is<dom::ObjectValue>()) {
    return memm_->read(v.template as<dom::ObjectValue>(), i);
  }
  return std::nullopt;
}

template <typename T>
bool Storage::set(dom::ObjectValue &obj, const std::string_view key,
                  T val) const {
  if (!obj.is<dom::ObjectValue>()) {
    return false;
  }
  return memm_->write(obj, key, val);
}
template <Derived<dom::ObjectValue>>
std::unique_ptr<dom::ObjectValue>
Storage::set(dom::ObjectValue &obj, std::string_view key, size_t size) const {
  if (!obj.is<dom::ObjectValue>()) {
    return false;
  }
  return memm_->write<dom::ObjectValue>(obj, key, size);
}
