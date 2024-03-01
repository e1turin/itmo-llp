
#include "store.h"

Response Create(const Storage &st, Query *q, const Storage::Data &d) {
  Query *curr = q;
  std::optional<Node> n;
  std::optional<Storage::Data> result;
  while (!curr->is_terminal()) {
    n = st.root();
    switch (curr->type) {
    case Query::kSelect:
      n = st.get(*n, std::get<std::string>(curr->value));
      break;
    case Query::kInsert:
      n = st.set(*n, std::get<std::string>(curr->value), d);
      break;
    case Query::kDelete: result = st.truncate(*n); break;
    case Query::kUpdate:
      n = st.set(*n, std::get<std::string>(curr->value), d);
      break;
    }
    curr = q->sub_query;
  }
  if (!n) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't access node"));
  }
  result = st.read(*n);
  if (!result) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't result data"));
  }
  return Response(Response::Status::kSuccess, Response::Payload(*result));
}
/**
 * On my point of view and scheme-less storage architecture, Create request is
 * same thing as Update request.
 */
Response Update(const Storage &st, Query *q, const Storage::Data& d) {
  return Create(st, q, d);
}
Response Remove(const Storage &st, Query *q) {
  Query *curr = q;
  std::optional<Node> n;
  std::optional<Storage::Data> result;
  while (!curr->is_terminal()) {
    n = st.root();
    switch (curr->type) {
    case Query::kSelect:
      n = st.get(*n, std::get<std::string>(curr->value));
      break;
    case Query::kInsert:
      return Response(Response::Status::kFail,
                      Response::Payload("Bad request"));
    case Query::kDelete: result = st.truncate(*n); break;
    case Query::kUpdate:
      return Response(Response::Status::kFail,
                      Response::Payload("Bad request"));
    }
    curr = q->sub_query;
  }
  if (!n) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't access node"));
  }
  result = st.read(*n);
  if (!result) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't result data"));
  }
  return Response(Response::Status::kSuccess, Response::Payload(*result));
}
Response Select(const Storage &st, Query *q) {
  Query *curr = q;
  std::optional<Node> n;
  std::optional<Storage::Data> result;
  while (!curr->is_terminal()) {
    n = st.root();
    switch (curr->type) {
    case Query::kSelect:
      n = st.get(*n, std::get<std::string>(curr->value));
      break;
    case Query::kInsert:
    case Query::kDelete:
    case Query::kUpdate:
      return Response(Response::Status::kFail,
                      Response::Payload("Bad request"));
    }
    curr = q->sub_query;
  }
  if (!n) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't access node"));
  }
  result = st.read(*n);
  if (!result) {
    return Response(Response::Status::kFail,
                    Response::Payload("Can't result data"));
  }
  return Response(Response::Status::kSuccess, Response::Payload(*result));
}
