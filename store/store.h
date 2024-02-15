#pragma once

#include "api/messages.h"
#include "api/storage.h"


Response Create(const Storage &, Query *, const Storage::Data&);
Response Update(const Storage &, Query *, const Storage::Data&);
Response Remove(const Storage &, Query *);
Response Select(const Storage &, Query *);
