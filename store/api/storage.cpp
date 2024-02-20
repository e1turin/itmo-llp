#include "storage.h"

#include <format>

Storage::Storage(std::string_view file_name) {
  auto db_file = fs::File(file_name);

  memm_ = std::make_unique<mem::MemoryManager>(std::move(db_file));
}

std::optional<dom::Value> Storage::root() const { return memm_->get_root(); }

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
  std::optional<std::vector<char>> chars =
      memm_->read_all<char>(mem::Offset{v.get_ref()});
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
      memm_->read_all<dom::Entry>(mem::Offset{v.get_ref()});
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
      memm_->read_all<dom::Entry>(mem::Offset{obj.get_ref()});
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

bool Storage::set(dom::ObjectValue &obj, std::string_view key, bool val) const {
  std::function<int(size_t, dom::Entry *)> func =
      [&](size_t size, dom::Entry *data) -> int {
    for (size_t i = 0; i < size; ++i) {
      std::optional<std::string_view> k = read(data[i].key);
      if (!k.has_value()) {
        return -1;
      }
      if (k == key) {
        //TODO: chck node type
        data[i].value = dom::BoolValue(val).as<dom::Value>();
        return 1;
      }
    }
    return 0;
  };
  std::optional<int> res =
      memm_->do_for_entries(mem::Offset{obj.get_ref()}, func);

  if (!res.has_value()) {
    return false;
  }
  if (res.value() == 1) {
    return true;
  }
  std::optional<std::vector<dom::Entry>> data = read(obj);
  if(!data.has_value()) {
    return false;
  }
  mem::Offset str = memm_->alloc<char>(key.size());
  bool key_res = memm_->write(str, key.data(), key.size());
  if (!key_res) {
    return false;
  }
  data->push_back({.key   = dom::StringValue(str),
                   .value = dom::BoolValue(val).as<dom::Value>()});
  mem::Offset entries = memm_->alloc<dom::Entry>(data->size());
  return memm_->write(entries, data->data(), data->size());
}

/* Storage::set overloads */

/* Storage::truncate overloads */
