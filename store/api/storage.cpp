#include "storage.h"

#include <format>
#include <iostream>

Storage::Storage(std::string_view file_name) {
  auto db_file = fs::File(file_name);

  memm_ = std::make_unique<mem::MemoryManager>(std::move(db_file));
}

std::optional<Node> Storage::root() const {
  if (auto ref = memm_->root_ref(); ref.has_value()) {
    return Node{ref.value()};
  }
  return std::nullopt;
}

std::optional<dom::Value::Type> Storage::get_type(Node node) const {
  mem::Offset off = node.get_ref();
  std::optional<dom::Value> value = memm_->read<dom::Value>(off);
  if (!value.has_value()) {
    return std::nullopt;
  }
  return value->get_type();
}

std::optional<Storage::Data> Storage::read(Node n) {
  auto value = memm_->read<dom::Value>(n.get_ref());
  if (!value.has_value()) {
    return std::nullopt;
  }
  switch ((*value).get_type()) {
  case dom::Value::Type::kBoolean: return read(value->as<dom::BoolValue>());
  case dom::Value::Type::kInt32:   return read(value->as<dom::Int32Value>());
  case dom::Value::Type::kFloat32: return read(value->as<dom::Float32Value>());
  case dom::Value::Type::kString:  return read(value->as<dom::StringValue>());
  case dom::Value::Type::kObject:  // explicit method
  case dom::Value::Type::kNull:    // no content
  default:                         return std::nullopt;
  }
}

std::optional<Storage::ObjEntries> Storage::get_entries(Node n) {
  auto value = memm_->read<dom::Value>(n.get_ref());
  if (!value.has_value() || (*value).get_type() != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  //TODO
  //  memm_->ref_for_all_pairs_of<dom::Value>()
}

std::optional<Node> Storage::get(Node n, std::string_view k) {
  if (get_type(n) != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  std::function<dom::Value *(size_t, dom::Entry *)> same_key =
      [&](size_t size, dom::Entry *ent) -> dom::Value * {
    for(size_t i = 0; i < size; ++i) {
      auto &e = ent[i];
      if (auto str = read(e.key);
          str.has_value() && str.value() == k) {
        return &ent[i].value;
      }
    }
    return nullptr;
  };
  std::optional<mem::Offset> off = memm_->find_in_entries(n.get_ref(), same_key);
  if (!off.has_value()) {
    return std::nullopt;
  }
  return Node{off.value()};
}

std::optional<Node>
Storage::set(Node n, std::string_view k, const Data& d) {
  if (auto t = get_type(n);
      !t.has_value() || *t != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  std::optional<Node> value = get(n, k);
  if (!value.has_value()) {
    return std::nullopt;
  }

  if (!truncate(*value)) {
    std::cerr << "Can't truncate vale on " << (*value).get_ref().value()
              << std::endl;
    return std::nullopt;
  }

  if (std::holds_alternative<std::string_view>(d)) {
    std::string_view str = std::get<std::string_view>(d);
    mem::Offset offset   = memm_->alloc<char>(str.size());
    if (!memm_->write(offset, str.data(), str.size()) ||
        !memm_->write(value->get_ref(), dom::StringValue(offset))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<bool>(d)) {
    bool b = std::get<bool>(d);
    if (!memm_->write(value->get_ref(), dom::BoolValue(b))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<std::int32_t >(d)) {
    std::int32_t i = std::get<std::int32_t>(d);
    if (!memm_->write(value->get_ref(), dom::Int32Value(i))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<float>(d)) {
    float f = std::get<float>(d);
    if(!memm_->write(value->get_ref(), dom::Float32Value(f))) {
      return std::nullopt;
    }
  } else {
    std::cerr << "Undefined value variant" << std::endl;
    return std::nullopt;
  }
  return value.value();
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


/*   *//* DEPRECATED *//*
std::optional<dom::Value> Storage::get(const dom::ObjectValue &obj,
                                       const std::string_view key) const {
  if (!obj.is<dom::ObjectValue>()) {
    return std::nullopt;
  }
  *//* todo optimize with additional dangerous method accessed mapped memory *//*

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
  memm_->free(mem::Offset{obj.get_ref()});
  mem::Offset entries = memm_->alloc<dom::Entry>(data->size());
  //TODO: update parent object ref...
  return memm_->write(entries, data->data(), data->size());
}
*/

/* Storage::set overloads */

/* Storage::truncate overloads */
