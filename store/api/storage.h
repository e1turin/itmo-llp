#pragma once
#include <store/dom/value.h>
#include <store/memory/memory_manager.h>

class Storage final {
public:
  explicit Storage(std::string_view);

  dom::Value root();

  template <typename T>
  T read(dom::Value);

  template <typename V, typename K>
  dom::Value get(V, K);

  template <typename V, typename K, typename T>
  dom::Value set(V, K, T);

  template <typename K>
  bool trancate(dom::ObjectValue, K);

private:
  std::unique_ptr<mem::MemoryManager> memm_;
};


template <typename T>
T Storage::read(dom::Value) {}

template <typename V, typename K>
dom::Value Storage::get(V, K) {}

template <typename V, typename K, typename T>
dom::Value Storage::set(V, K, T) {}

template <typename K>
bool Storage::trancate(dom::ObjectValue, K) {}
