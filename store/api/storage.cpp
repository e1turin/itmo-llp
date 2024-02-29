#include "storage.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <memory>

Storage::Storage(std::string_view file_name, bool replace_if_exists) {
  auto f = std::make_unique<fs::File>(file_name);
  memm_ = std::make_unique<mem::MemoryManager>(std::move(f), replace_if_exists);
}

std::optional<Node> Storage::root() const {
  std::optional<mem::Offset> ref = memm_->root_ref();
  return ref ? std::make_optional<Node>(*ref) : std::nullopt;
}

std::optional<dom::Value::Type> Storage::get_type(Node node) const {
  std::optional<dom::Value> value = memm_->read<dom::Value>(node.get_ref());
  return value ? std::make_optional(value->get_type()) : std::nullopt;
}

std::optional<Storage::Data> Storage::read(Node node) {
  std::optional<dom::Value> value = memm_->read<dom::Value>(node.get_ref());
  if (!value) {
    return std::nullopt;
  }
  switch (value->get_type()) {
  case dom::Value::Type::kBoolean: return read(value->as<dom::BoolValue>());
  case dom::Value::Type::kInt32:   return read(value->as<dom::Int32Value>());
  case dom::Value::Type::kFloat32: return read(value->as<dom::Float32Value>());
  case dom::Value::Type::kString:  return read(value->as<dom::StringValue>());
  case dom::Value::Type::kObject:  // explicit method
  case dom::Value::Type::kNull:    // no content
  default:                         return std::nullopt;
  }
}

std::optional<Storage::ObjEntries> Storage::get_entries(Node node) {
  std::optional<dom::Value> value = memm_->read<dom::Value>(node.get_ref());
  if (!value || value->get_type() != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  auto data = value->as<dom::ObjectValue>().get_ref();
  std::optional<std::vector<std::pair<mem::Offset, mem::Offset>>> data_refs =
      memm_->ref_all_pairs<dom::Value>(data);
  if (!data_refs) {
    return std::nullopt;
  }
  std::vector<std::pair<Node, Node>> nodes;
  std::transform(data_refs->begin(), data_refs->end(),
                 std::back_inserter(nodes),
                 [](const std::pair<mem::Offset, mem::Offset> &p)
                     -> std::pair<Node, Node> {
                   return std::make_pair(Node{p.first}, Node{p.second});
                 });
  return nodes;
}

std::optional<Node> Storage::get(Node node, std::string_view key) {
  std::optional<dom::Value> value = memm_->read<dom::Value>(node.get_ref());
  if (!value || value->get_type() != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  if (value->as<dom::ObjectValue>().get_ref().value() == 0) {
    return Node{mem::Offset{0}};
  }
  std::function<dom::Value *(size_t, dom::Entry *)> same_key =
      [&](size_t size, dom::Entry *ent) -> dom::Value * {
        for (size_t i = 0; i < size; ++i) {
          auto &e = ent[i];
          if (auto str = read(e.key); str && *str == key) {
            return &ent[i].value;
          }
        }
        return nullptr;
      };
  std::optional<mem::Offset> ref =
      memm_->find_in_entries(value->as<dom::ObjectValue>().get_ref(), same_key);
  // null-ref is 'not found' result, no need to handle
  return ref ? std::make_optional<Node>(*ref) : std::nullopt;
}

std::optional<Node> Storage::set(Node node, std::string_view key,
                                 const Data &data) {
  std::optional<Node> value = get(node, key);
  if (!value) {
    return std::nullopt;
  }
  if (value->get_ref().value() == 0) {
    std::optional<Node> new_value = insert_key(node, key);
    if (!new_value) {
      return std::nullopt;
    }
    value = new_value;
  } else if (!truncate(*value)) {
    std::cerr << "Can't truncate value on offset=" << value->get_ref().value()
              << std::endl;
    return std::nullopt;
  }
  if (std::holds_alternative<std::string_view>(data)) {
    std::string_view str = std::get<std::string_view>(data);
    mem::Offset offset   = memm_->alloc<char>(str.size());
    if (!memm_->write(offset.after<size_t>(), str.data(), str.size()) ||
        !memm_->write(value->get_ref(), dom::StringValue(offset))) {
      memm_->free(offset);
      return std::nullopt;
    }
  } else if (std::holds_alternative<bool>(data)) {
    bool b = std::get<bool>(data);
    if (!memm_->write(value->get_ref(), dom::BoolValue(b))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<std::int32_t>(data)) {
    std::int32_t i = std::get<std::int32_t>(data);
    if (!memm_->write(value->get_ref(), dom::Int32Value(i))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<float>(data)) {
    float f = std::get<float>(data);
    if (!memm_->write(value->get_ref(), dom::Float32Value(f))) {
      return std::nullopt;
    }
  } else if (std::holds_alternative<std::nullptr_t>(data)) {
    if (!memm_->write(value->get_ref(), dom::ObjectValue::null_object())) {
      return std::nullopt;
    }
  } else {
    std::cerr << "Undefined value variant" << std::endl;
    return std::nullopt;
  }
  return *value;
}

bool Storage::truncate(Node node) {
  /* TODO: change recursion to a linear tree traversal */
  std::optional<dom::Value> val = memm_->read<dom::Value>(node.get_ref());
  if (!val) {
    return false;
  }
  auto type = val->get_type();
  if (type == dom::Value::Type::kString) {
    mem::Offset chars = val->as<dom::StringValue>().get_ref();
    if (!memm_->free(chars)) {
      return false;
    }
  } else if (type == dom::Value::Type::kObject) {
    mem::Offset data = val->as<dom::ObjectValue>().get_ref();
    if (data.value() == 0) {
      return true; // already null-object == empty
    }
    // todo: change on MemoryManager::do_for_entries(..., [](){...truncate})
    std::optional<ObjEntries> children = get_entries(node);
    if (!children) {
      return false;
    }
    for (auto &p : *children) {
      if (!truncate(p.first) || !truncate(p.second)) {
        return false;
      }
    }
    if (!memm_->free(data)) {
      return false;
    }
  }
  return memm_->write(node.get_ref(), dom::ObjectValue::null_object());
}

bool Storage::truncate(Node node, std::string_view key) {
  std::optional<dom::Value> val = memm_->read<dom::Value>(node.get_ref());
  if (val->get_type() != dom::Value::Type::kObject) {
    return false;
  }
  mem::Offset data_ref = val->as<dom::ObjectValue>().get_ref();
  if (data_ref.value() == 0) { // null-object is already truncated
    return true;
  }
  std::function<std::vector<dom::Value *>(size_t, dom::Entry *)> key_and_value =
      [&](size_t size, dom::Entry *ent) -> std::vector<dom::Value *> {
    std::vector<dom::Value *> values;
    for (size_t i = 0; i < size; ++i) {
      auto &e = ent[i];
      if (auto str = read(e.key); str && *str == key) {
        values.push_back(&e.key);
        values.push_back(&e.value);
        auto &end = ent[size - 1];
        values.push_back(&end.key);
        values.push_back(&end.value);
        return values;
      }
    };
    return values;
  };
  std::optional<std::vector<mem::Offset>> entry =
      memm_->find_in_entries(data_ref, key_and_value);
  if (!entry) {
    return false;
  }
  if (entry->empty()) { // No such key -> truncation complete.
    return true;
  }
  if (entry->size() != 4) { // Found required key-value and last
    return false;           // key-value for substitution.
  }
  Node key_node        = Node{(*entry)[0]};
  Node value_node      = Node{(*entry)[1]};
  Node last_key_node   = Node{(*entry)[2]};
  Node last_value_node = Node{(*entry)[3]};
  if (!truncate(key_node) || !truncate(value_node)) {
    return false;
  }
  std::optional<size_t> count_entries = memm_->read<size_t>(data_ref);
  if (!count_entries) {
    return false;
  }
  if (!memm_->write(data_ref, *count_entries - 1)) {
    return false;
  }
  std::optional<dom::Value> last_key =
      memm_->read<dom::Value>(last_key_node.get_ref());
  if (!last_key) {
    return false;
  }
  std::optional<dom::Value> last_value =
      memm_->read<dom::Value>(last_value_node.get_ref());
  if (!last_value) {
    return false;
  }
  return memm_->write(last_key_node.get_ref(), std::move(*last_key)) &&
         memm_->write(last_value_node.get_ref(), std::move(*last_value));
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
  if (!chars) {
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
  return *chars;
}

std::optional<Node> Storage::insert_key(Node node, std::string_view key) const {
  std::optional<dom::Value> value = memm_->read<dom::Value>(node.get_ref());
  if (!value || value->get_type() != dom::Value::Type::kObject) {
    return std::nullopt;
  }
  if (value->as<dom::ObjectValue>().get_ref().value() == 0) {
    constexpr size_t some_num_of_entries = 4;
    mem::Offset data = memm_->alloc<dom::Entry>(some_num_of_entries, false);
    if (!memm_->write(node.get_ref(), dom::ObjectValue{data}))  {
      memm_->free(data);
      return std::nullopt;
    }
  }
  value = memm_->read<dom::Value>(node.get_ref());
  mem::Offset old_data_ref = value->as<dom::ObjectValue>().get_ref();
  std::optional<size_t> size =
      memm_->read<size_t>(old_data_ref);
  if (!size) {
    return std::nullopt;
  }
  mem::Offset new_data_ref = memm_->realloc<dom::Entry>(old_data_ref, *size + 1);
  if (new_data_ref.value() != old_data_ref.value() &&
      (!memm_->move<dom::Entry>(new_data_ref, old_data_ref, *size) ||
       !memm_->free(old_data_ref))) {
    return std::nullopt;
  }
  std::function<std::vector<dom::Value *>(size_t, dom::Entry *)>
      last_key_and_value =
          [](size_t size, dom::Entry *ent) -> std::vector<dom::Value *> {
    std::vector<dom::Value *> key_and_value;
    // We have enough space for our purpose, so
    // just take elements after allocated ones.
    dom::Entry &end = ent[size];
    key_and_value.push_back(&end.key);
    key_and_value.push_back(&end.value);
    return key_and_value;
  };
  std::optional<std::vector<mem::Offset>> entry =
      memm_->find_in_entries(new_data_ref, last_key_and_value);
  if (!entry) {
    return std::nullopt;
  }
  Node last_key        = Node{(*entry)[0]};
  Node last_value      = Node{(*entry)[1]};
  mem::Offset str_data = memm_->alloc<char>(key.size());
  if (!memm_->write(str_data.after<size_t>(), key.data(), key.size()) ||
      !memm_->write(last_key.get_ref(), dom::StringValue{str_data}) ||
      !memm_->write(last_value.get_ref(), dom::ObjectValue::null_object())) {
    memm_->free(str_data.before<size_t>());
    return std::nullopt;
  }
  return last_value;
}
