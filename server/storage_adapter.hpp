#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

extern "C" {
#include "element_storage_api.h"
#include "query.h"
}

struct query_result;

class StorageAdapter {
 public:
  StorageAdapter(const std::string& filename, size_t page_size = 4096);

  StorageAdapter(const StorageAdapter&) = delete;
  StorageAdapter(StorageAdapter&&) = delete;

  ~StorageAdapter();

  bool processJsonQuery(const nlohmann::json& query);

  void outputResultAsDot(std::ostream& out);
  nlohmann::json outputResultAsJson();

 private:
  void fillElementType(element_type** type, bool owns_type_ptr);
  void fillLinkType(link_type** type);
  void fillLinkPatternType(link_pattern& pattern);
  void fillElementPatternType(element_pattern& pattern);
  void fillQueryTypes(struct query& q);
  void fillTypesMap();

  std::map<std::string, link_type> link_types_map;
  link_type_id_t link_type_id;

  std::map<std::string, element_type> element_types_map;
  element_type_id_t element_type_id;

  element_storage* storage;
  struct query_result* last_query_result = NULL;
  bool has_result = false;
};