#pragma once

#include "api/messages.h"
#include "api/storage.h"


Response Create(const Storage &, const Query &, Storage::Data);
Response Update(const Storage &, const Query &, Storage::Data);
Response Remove(const Storage &, const Query &);
Response Select(const Storage &, const Query &);
