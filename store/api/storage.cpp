#include "storage.h"

#include <format>

Storage::Storage(std::string_view file_name) {
  auto db_file = fs::File(file_name);

  memm_ = std::make_unique<mem::MemoryManager>(std::move(db_file));
}

std::optional<dom::Value> Storage::root() const {
  return memm_->read<dom::Value>(fs::Offset(0));
}

/* Storage::read overloads */

std::optional<std::int32_t> Storage::read(const dom::Int32Value &v) {
  if (!v.is<dom::Int32Value>()) {
    return std::nullopt;
  }
  return v.get_int();
}

std::optional<float> Storage::read(const dom::Float32Value &v) {
  if (!v.is<dom::Float32Value>()) {
    return std::nullopt;
  }
  return v.get_float();
}

std::optional<bool> Storage::read(const dom::BoolValue &v) {
  if (!v.is<dom::BoolValue>()) {
    return std::nullopt;
  }
  return v.get_bool();
}

std::optional<std::string_view> Storage::read(const dom::StringValue &v) const {
  if (!v.is<dom::StringValue>()) {
    return std::nullopt;
  }
  std::optional<std::vector<char>> chars = memm_->read_all<char>(v.get_ref());
  if (!chars.has_value()) {
    return std::nullopt;
  }
  return std::string_view{chars->data(), chars->size()};
}

std::optional<std::vector<dom::Entry>>
Storage::read(const dom::ObjectValue &v) const {
  if (!v.is<dom::ObjectValue>()) {
    return std::nullopt;
  }
  std::optional<std::vector<dom::Entry>> chars =
      memm_->read_all<dom::Entry>(v.get_ref());
  return std::move(chars);
}

/* Storage::get overloads */

std::optional<dom::Value> Storage::get(const dom::ObjectValue &obj,
                                       const std::string_view key) const {
  if (!obj.is<dom::ObjectValue>()) {
    return std::nullopt;
  }
  /* todo optimize with additional dangerous method accessed mapped memory */
  std::optional<std::vector<dom::Entry>> entries =
      memm_->read_all<dom::Entry>(obj.get_ref());
  if (!entries.has_value()) {
    return std::nullopt;
  }
  auto entry =
      std::find_if(entries->begin(), entries->end(), [&](dom::Entry &it) {
        std::optional<std::string_view> str = read(it.key);
        return str.has_value() && str == key;
      });
  if (entry == entries->end()) {
    return std::nullopt;
  }
  return entry->value;
}

/* Storage::set overloads */

/* Storage::truncate overloads */
