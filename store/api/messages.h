#pragma once

#include "storage.h"

struct Query {
  enum Type { kSelect, kInsert, kDelete, kUpdate };
  enum Constraint { kGreater, kLess, kEqual };

  Type type;
  Constraint cnrt;
  Storage::Data value;
};

struct Response {
  enum Status { kSuccess, kFail };

  Status status;
  union {
    Storage::Data data;
    std::string msg;
  };
};

