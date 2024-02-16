#include "storage.h"

#include <format>

Storage::Storage(std::string_view file_name) {
  auto db_file = fs::File(file_name);

  memm_ = std::make_unique<mem::MemoryManager>(std::move(db_file));
}

std::unique_ptr<dom::Value> Storage::root() const {
  return std::move(memm_->read(fs::Offset(0)));
}

/* Storage::read overloads */

std::optional<std::int32_t> Storage::read(const dom::Int32Value &v) const {
  if (!v.is<dom::Int32Value>()) {
    return std::nullopt;
  }
  return memm_->read(v);
}

std::optional<float> Storage::read(const dom::Float32Value &v) const {
  if (!v.is<dom::Float32Value>()) {
    return std::nullopt;
  }
  return memm_->read(v);
}

std::optional<bool> Storage::read(const dom::BoolValue &v) const {
  if (!v.is<dom::BoolValue>()) {
    return std::nullopt;
  }
  return memm_->read(v);
}

std::optional<std::string_view> Storage::read(const dom::StringValue &v) const {
  if (!v.is<dom::StringValue>()) {
    return std::nullopt;
  }
  return memm_->read(v);
}

std::unique_ptr<std::vector<dom::Entry>>
Storage::read(const dom::ObjectValue &v) const {
  if (!v.is<dom::ObjectValue>()) {
    return nullptr;
  }
  return std::move(memm_->read(v));
}

/* Storage::get overloads */

std::unique_ptr<dom::Value> Storage::get(const dom::ObjectValue &obj,
                                         const std::string_view key) const {
  if (!obj.is<dom::ObjectValue>()) {
    return nullptr;
  }
  return memm_->read(obj, key);
}

/* Storage::set overloads */

size_t Storage::set(dom::ObjectValue &obj, const std::string_view key,
                    const std::string_view val) const {
  if (!obj.is<dom::ObjectValue>()) {
    return -1;
  }
  return memm_->write(obj, key, val);
}

/* Store::trancate overloads */

bool Storage::trancate(dom::ObjectValue &obj, const size_t idx) const {
  if (!obj.is<dom::ObjectValue>()) {
    return false;
  }
  return memm_->free(obj, idx) != 0;
}

bool Storage::trancate(dom::ObjectValue &obj, const std::string_view key) const {
  if (!obj.is<dom::ObjectValue>()) {
    return false;
  }
  return memm_->free(obj, key) != 0;
}
