#pragma once

#include "store/memory/offset.h"
#include "store/dom/value.h"

/**
 * Reference to Value in tree.
 */
class Node {
public:
  explicit Node(mem::Offset ref) : ref_(ref) {}
  [[nodiscard]] mem::Offset get_ref() const {
    return ref_;
  }
  [[nodiscard]] bool is_null() const {
    return ref_.value() == 0;
  }
private:
  mem::Offset ref_;
};
