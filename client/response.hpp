#pragma once

extern "C" {
#include "attribute.h"
}

#include <iostream>
#include <string>
#include <vector>

struct Element {
  std::string type;
  std::vector<attribute> attrs;
};

struct Response {
  int error;

  std::vector<Element> elements;

  void dump(std::ostream& out);
};