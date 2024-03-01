#pragma once

#include "storage.h"

struct Query {
  enum Type { kSelect, kInsert, kDelete, kUpdate };
  enum Constraint { kGreater, kLess, kEqual };

  Type type;
  Constraint cnrt;
  Storage::Data value;
  Query *sub_query;

  [[nodiscard]] bool is_terminal() const {
    return type != kSelect;
  }
};

struct Response {
  enum Status { kSuccess, kFail };
  union Payload {
    Storage::Data data;
    std::string msg;
    ~Payload() {}
  };

  Status status;
  Payload payload;

//  Response(Status status, std::string str) : status(status), payload(str) {}
//  Response(Status status, Sto) : status(status), payload(str) {}
};

