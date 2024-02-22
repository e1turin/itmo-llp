#pragma once

#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_

#include "inttypes.h"
#include "stdbool.h"

enum ATTRIBUTE_TYPE { ATTR_TYPE_INT32 = 0, ATTR_TYPE_INT64, ATTR_TYPE_BOOL, ATTR_TYPE_DOUBLE, ATTR_TYPE_STRING };

struct attribute {
  enum ATTRIBUTE_TYPE type;

  union {
    struct {
      char* data;
    } as_string;

    double as_double;
    bool as_bool;
    int32_t as_int32;
    int64_t as_int64;
  } value;
};

#endif /* _ATTRIBUTE_H_ */