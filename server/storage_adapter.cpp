#include "storage_adapter.hpp"

#include <iostream>

#include "json_to_database_query.hpp"
#include "result_to_json.hpp"

extern "C" {
#include "query_processor.h"
}

int dump_element(std::ostream& out, element* el, int number) {
  if (!el) {
    out << "NULL" << std::endl;
    return number;
  }
  std::string label = std::string(el->element_type->type_name);
  for (size_t i = 0; i < el->attribute_count; ++i) {
    label += "\\n";
    switch (el->attributes[i].type) {
      case ATTR_TYPE_INT32:
        label += std::to_string(el->attributes[i].value.as_int32);
        break;
      case ATTR_TYPE_INT64:
        label += std::to_string(el->attributes[i].value.as_int64);
        break;
      case ATTR_TYPE_BOOL:
        label += std::to_string(el->attributes[i].value.as_bool);
        break;
      case ATTR_TYPE_STRING:
        label += std::string(el->attributes[i].value.as_string.data);
        break;
      case ATTR_TYPE_DOUBLE:
        label += std::to_string(el->attributes[i].value.as_double);
        break;
      default:
        label += "UNKNOWN TYPE";
        break;
    }
  }
  out << number << " [label = \"" << label << "\"];" << std::endl;

  int root_number = number;
  for (size_t i = 0; i < el->links_in_cnt; ++i) {
    number = dump_element(out, el->links_in[i].dst, number);
    out << number << "->" << root_number << "[label = \"" << el->links_in[i].link_type << "\"]" << std::endl;
  }

  for (size_t i = 0; i < el->links_out_cnt; ++i) {
    number = dump_element(out, el->links_out[i].dst, number);
    out << number << "<-" << root_number << "[label = \"" << el->links_out[i].link_type << "\"]" << std::endl;
  }
  return number + 1;
}

StorageAdapter::StorageAdapter(const std::string& filename, size_t page_size) {
  element_storage_alloc(&storage);

  bool exists = false;
  if (FILE* file = fopen(filename.c_str(), "r")) {
    fclose(file);
    exists = true;
  }

  if (exists) {
    element_storage_read(storage, filename.c_str(), page_size);
  } else {
    element_storage_ctor(storage, filename.c_str(), page_size);
  }

  fillTypesMap();
  query_result_alloc(&last_query_result);
}

StorageAdapter::~StorageAdapter() {
  element_storage_dtor(storage);
  free(storage);
  for (auto& p : element_types_map) {
    element_type_dtor(&p.second);
  }
  for (auto& p : link_types_map) {
    link_type_dtor(&p.second);
  }

  free(last_query_result);
}

void dump_elements(element_storage* storage, storage_iterator* element_type_iter, element_type& type) {
  std::cout << "FOUND " << type.type_id << " " << type.type_name << std::endl;
  struct storage_iterator* el_iter;
  storage_iterator_alloc(&el_iter);
  struct storage_iterator* el_start_iter;
  storage_iterator_alloc(&el_start_iter);

  error_code err = element_storage_get_element_iter(storage, element_type_iter, el_iter);
  storage_iterator_copy(el_iter, el_start_iter);

  if (err == 0) {
    int number = 1;
    struct element el = {0};
    do {
      element_storage_get_element(storage, el_iter, &el);

      std::cout << "FOUND" << std::endl;
      number = dump_element(std::cout, &el, number);
      std::cout << "DNUOF" << std::endl;

      element_dtor(&el);

      storage_iterator_next(el_iter);
    } while (!storage_iterator_equals(el_iter, el_start_iter));
  } else {
    if (err == ERROR_LIST_IS_EMPTY) {
      std::cout << "Element List is empty!" << std::endl;
    } else {
      std::cout << "Error occurred " << err << std::endl;
    }
  }
}

void StorageAdapter::fillTypesMap() {
  for (auto& p : element_types_map) {
    element_type_dtor(&p.second);
  }
  element_types_map.clear();
  for (auto& p : link_types_map) {
    link_type_dtor(&p.second);
  }
  link_types_map.clear();

  struct storage_iterator* type_iter;
  storage_iterator_alloc(&type_iter);

  struct storage_iterator* start_iter;
  storage_iterator_alloc(&start_iter);

  if (!element_storage_element_type_list_is_empty(storage)) {
    element_storage_get_element_type_iter(storage, type_iter);
    storage_iterator_copy(type_iter, start_iter);

    element_type element_type;
    do {
      element_storage_get_element_type(storage, type_iter, &element_type);

      element_type_id = std::max(element_type_id, element_type.type_id);

      // dump_elements(storage, type_iter, element_type);

      element_types_map.emplace(std::string(element_type.type_name), element_type);

      storage_iterator_next(type_iter);
    } while (!storage_iterator_equals(type_iter, start_iter));

    ++element_type_id;
  } else {
    element_type_id = 1;
  }

  if (!element_storage_link_type_list_is_empty(storage)) {
    element_storage_get_link_type_iter(storage, type_iter);
    storage_iterator_copy(type_iter, start_iter);

    link_type link_type;
    do {
      element_storage_get_link_type(storage, type_iter, &link_type);

      link_type_id = std::max(link_type_id, link_type.type_id);

      link_types_map.emplace(std::string(link_type.type_name), link_type);

      storage_iterator_next(type_iter);
    } while (!storage_iterator_equals(type_iter, start_iter));

    ++link_type_id;
  } else {
    link_type_id = 1;
  }

  free(start_iter);
  free(type_iter);
}

void StorageAdapter::fillElementType(element_type** type, bool owns_type_ptr) {
  element_type* ptr = (*type);
  *type = &element_types_map[ptr->type_name];

  if (owns_type_ptr) {
    if (ptr) {
      element_type_dtor(ptr);
    }
    free(ptr);
  }
}

void StorageAdapter::fillLinkType(link_type** type) {
  link_type* ptr = *type;
  *type = &link_types_map[ptr->type_name];
  link_type_dtor(ptr);
  free(ptr);
}

void StorageAdapter::fillLinkPatternType(link_pattern& pattern) {
  fillLinkType(&(pattern.link_type));
  if (pattern.target) {
    fillElementPatternType(*(pattern.target));
  }
}

void StorageAdapter::fillElementPatternType(element_pattern& pattern) {
  fillElementType(&(pattern.element_type), true);
  for (size_t i = 0; i < pattern.links_in_cnt; ++i) {
    fillLinkPatternType(pattern.links_in[i]);
  }
  for (size_t i = 0; i < pattern.links_out_cnt; ++i) {
    fillLinkPatternType(pattern.links_out[i]);
  }
}

void StorageAdapter::fillQueryTypes(struct query& q) {
  switch (q.type) {
    case QUERY_TYPE_CREATE:
      fillElementType(&(q.args.as_create.element->element_type), q.args.as_create.element->owns_type_ptr);
      q.args.as_create.element->owns_type_ptr = false;
      break;
    case QUERY_TYPE_DELETE:
      fillElementPatternType(*(q.args.as_delete.pattern));
      break;
    case QUERY_TYPE_MATCH:
      fillElementPatternType(*(q.args.as_match.pattern));
      break;
    case QUERY_TYPE_LINK:
      fillElementPatternType(*(q.args.as_link.source_pattern));
      fillElementPatternType(*(q.args.as_link.dst_pattern));
      fillLinkType(&(q.args.as_link.link_type));
      break;
    case QUERY_TYPE_SET:
      fillElementPatternType(*(q.args.as_set.pattern));
      break;
    default:
      break;
  }
}

void dump_attr_val(std::ostream& out, const attribute& attr) {
  switch (attr.type) {
    case ATTR_TYPE_BOOL:
      out << "(bool) " << attr.value.as_bool << std::endl;
      break;
    case ATTR_TYPE_DOUBLE:
      out << "(double) " << attr.value.as_double << std::endl;
      break;
    case ATTR_TYPE_INT32:
      out << "(int32) " << attr.value.as_int32 << std::endl;
      break;
    case ATTR_TYPE_INT64:
      out << "(int64) " << attr.value.as_int64 << std::endl;
      break;
    case ATTR_TYPE_STRING:
      out << "(str) " << attr.value.as_string.data << std::endl;
      break;
    default:
      out << "Unknown attr type" << std::endl;
      break;
  }
}

void dump_attr_pattern(std::ostream& out, const attribute_pattern& pattern) {
  out << "Attribute pattern type: ";
  switch (pattern.node_type) {
    case ATTRIBUTE_PATTERN_BIN_OP: {
      out << "BIN_OP ";
      if (pattern.args.as_bin.op_type == ATTRIBUTE_PATTERN_AND) {
        out << "AND ";
      } else {
        out << "OR ";
      }

      out << std::endl;
      if (!pattern.args.as_bin.lhs) {
        out << "NULL";
      } else {
        dump_attr_pattern(out, *(pattern.args.as_bin.lhs));
        out << std::endl;
      }
      if (!pattern.args.as_bin.rhs) {
        out << "NULL";
      } else {
        dump_attr_pattern(out, *(pattern.args.as_bin.rhs));
      }
      break;
    }
    case ATTRIBUTE_PATTERN_UN_OP:
      out << "UN_OP NOT ";
      if (!pattern.args.as_unary.arg) {
        out << "NULL";
      } else {
        dump_attr_pattern(out, *(pattern.args.as_unary.arg));
      }
      break;
    case ATTRIBUTE_PATTERN_COND:
      out << "COND ";
      switch (pattern.args.as_cond.cond) {
        case ATTRIBUTE_PATTERN_COND_CONTAINS:
          out << "CONTAINS ";
          break;
        case ATTRIBUTE_PATTERN_COND_EQ:
          out << "EQ ";
          break;
        case ATTRIBUTE_PATTERN_COND_GREATER:
          out << "GT ";
          break;
        case ATTRIBUTE_PATTERN_COND_LESS:
          out << "LT ";
          break;
        default:
          out << "UNKNOWN ";
          break;
      }
      out << pattern.args.as_cond.attr_id << ' ';
      dump_attr_val(out, pattern.args.as_cond.attr);
      break;
    default:
      out << "UNKNOWN ";
      break;
  }
}

void dump_pattern(std::ostream& out, const element_pattern& pattern);

void dump_link(std::ostream& out, const link_pattern& pattern) {
  out << "Link type: " << pattern.link_type->type_name << std::endl;
  out << "Target: " << std::endl;
  if (!pattern.target) {
    out << "NULL";
  } else {
    dump_pattern(out, *(pattern.target));
  }
}

void dump_pattern(std::ostream& out, const element_pattern& pattern) {
  out << "Pattern of element type |" << pattern.element_type->type_name << "|" << std::endl;
  out << "Pattern of element type id |" << pattern.element_type->type_id << "|" << std::endl;
  out << "Pattern of element type attrs count |" << pattern.element_type->attribute_count << "|" << std::endl;
  if (pattern.attr_pattern) {
    dump_attr_pattern(out, *(pattern.attr_pattern));
  } else {
    out << "Attr pattern: NULL" << std::endl;
  }
  out << std::endl << "Links in: " << std::endl;
  for (size_t i = 0; i < pattern.links_in_cnt; ++i) {
    dump_link(out, pattern.links_in[i]);
    out << std::endl;
  }
  out << "Links out: " << std::endl;
  for (size_t i = 0; i < pattern.links_out_cnt; ++i) {
    dump_link(out, pattern.links_out[i]);
    out << std::endl;
  }
}

void dump_query(struct query& q) {
  switch (q.type) {
    case QUERY_TYPE_CREATE:
      std::cout << "Query create" << std::endl;
      std::cout << "Element: |" << q.args.as_create.element->element_type->type_name << '|' << std::endl;
      std::cout << "Attrs: " << q.args.as_create.element->attribute_count << std::endl;
      for (size_t i = 0; i < q.args.as_create.element->attribute_count; ++i) {
        std::cout << "Attr " << i << " = ";
        dump_attr_val(std::cout, q.args.as_create.element->attributes[i]);
      }
      break;
    case QUERY_TYPE_DELETE:
      std::cout << "Query delete" << std::endl;
      std::cout << "Pattern: " << std::endl;
      dump_pattern(std::cout, *(q.args.as_delete.pattern));
      std::cout << std::endl;
      break;
    case QUERY_TYPE_LINK:
      std::cout << "Query link" << std::endl;
      std::cout << "Link type: " << q.args.as_link.link_type->type_name << std::endl;
      std::cout << "Src: " << std::endl;
      dump_pattern(std::cout, *(q.args.as_link.source_pattern));
      std::cout << std::endl << "Dst: " << std::endl;
      dump_pattern(std::cout, *(q.args.as_link.dst_pattern));
      std::cout << std::endl;
      break;
    case QUERY_TYPE_MATCH:
      std::cout << "Query match" << std::endl;
      std::cout << "Pattern: " << std::endl;
      dump_pattern(std::cout, *(q.args.as_match.pattern));
      std::cout << std::endl;
      break;
    case QUERY_TYPE_SET:
      std::cout << "Query set" << std::endl;
      std::cout << "Pattern: " << std::endl;
      dump_pattern(std::cout, *(q.args.as_set.pattern));
      std::cout << "Attrs count: " << q.args.as_set.attributes_count << std::endl;
      for (size_t i = 0; i < q.args.as_set.attributes_count; ++i) {
        std::cout << "Attr: " << q.args.as_set.attributes[i].attribute_id << " = ";
        dump_attr_val(std::cout, q.args.as_set.attributes[i].attribute);
        std::cout << std::endl;
      }
      break;
    default:
      break;
  }
}

bool convertAttr(attribute& attr, ATTRIBUTE_TYPE type) {
  if (attr.type == type) {
    return true;
  }

  if (type == ATTR_TYPE_STRING || attr.type != ATTR_TYPE_INT64) {
    return false;
  }

  switch (type) {
    case ATTR_TYPE_INT32:
      if (attr.value.as_int64 <= std::numeric_limits<int32_t>::max() &&
          attr.value.as_int64 >= std::numeric_limits<int32_t>::min()) {
        attr.type = ATTR_TYPE_INT32;
      } else {
        return false;
      }
      break;
    case ATTR_TYPE_DOUBLE:
      attr.type = ATTR_TYPE_DOUBLE;
      attr.value.as_double = attr.value.as_int64;
      break;
    case ATTR_TYPE_BOOL:
      if (attr.value.as_int64 >= 0 && attr.value.as_int64 <= 1) {
        attr.type = ATTR_TYPE_BOOL;
      } else {
        return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool convertArgs(ATTRIBUTE_TYPE* types, attribute* attrs, attribute_count_t attr_count) {
  for (attribute_count_t i = 0; i < attr_count; ++i) {
    if (!convertAttr(attrs[i], types[i])) {
      return false;
    }
  }
  return true;
}

bool convertPatternArgs(attribute_pattern* pattern, ATTRIBUTE_TYPE* types, attribute_count_t attr_count) {
  if (!pattern) {
    return true;
  }

  switch (pattern->node_type) {
    case ATTRIBUTE_PATTERN_BIN_OP:
      if (!convertPatternArgs(pattern->args.as_bin.lhs, types, attr_count)) {
        return false;
      }
      return convertPatternArgs(pattern->args.as_bin.rhs, types, attr_count);
    case ATTRIBUTE_PATTERN_UN_OP:
      return convertPatternArgs(pattern->args.as_unary.arg, types, attr_count);
    case ATTRIBUTE_PATTERN_COND:
      return convertAttr(pattern->args.as_cond.attr, types[pattern->args.as_cond.attr_id]);
    default:
      return false;
  }
}

bool convertPatternArgs(element_pattern* pattern) {
  if (!pattern) {
    return true;
  }
  return convertPatternArgs(pattern->attr_pattern, pattern->element_type->attribute_types,
                            pattern->element_type->attribute_count);
}

// Use after filling the element types, tries to convert numerical args that are by default int64
bool validateOrConvertArgs(struct query& q) {
  switch (q.type) {
    case QUERY_TYPE_CREATE:
      return convertArgs(q.args.as_create.element->element_type->attribute_types, q.args.as_create.element->attributes,
                         q.args.as_create.element->attribute_count);
    case QUERY_TYPE_DELETE:
      return convertPatternArgs(q.args.as_delete.pattern);
    case QUERY_TYPE_MATCH:
      return convertPatternArgs(q.args.as_match.pattern);
    case QUERY_TYPE_SET:
      return convertPatternArgs(q.args.as_set.pattern);
    case QUERY_TYPE_LINK:
      if (!convertPatternArgs(q.args.as_link.source_pattern)) {
        return false;
      }
      return convertPatternArgs(q.args.as_link.dst_pattern);
    default:
      return true;
  }
}

bool StorageAdapter::processJsonQuery(const nlohmann::json& query) {
  struct query q;
  json_to_database_query(query, q);
  fillQueryTypes(q);
  if (!validateOrConvertArgs(q)) {
    std::cerr << "Invalid arguments types" << std::endl;
    return false;
  }

  if (q.type == QUERY_TYPE_CREATE_VERTEX_TYPE) {
    q.args.as_create_vertex_type.type->type_id = element_type_id++;
  }
  if (q.type == QUERY_TYPE_CREATE_EDGE_TYPE) {
    q.args.as_create_edge_type.type->type_id = link_type_id++;
  }

  if (q.type == QUERY_TYPE_MATCH) {
    has_result = true;
  } else {
    has_result = false;
  }

  // dump_query(q);
  error_code err = process_query(storage, &q, last_query_result);

  if (err != 0) {
    std::cerr << "ERROR OCCURRED DURING QUERY PROCESSING " << err << std::endl;
    has_result = false;
  } else {
    if (q.type == QUERY_TYPE_CREATE_VERTEX_TYPE || q.type == QUERY_TYPE_CREATE_EDGE_TYPE) {
      fillTypesMap();
    }
  }

  return (err == 0);
}

void StorageAdapter::outputResultAsDot(std::ostream& out) {
  if (!last_query_result || !has_result) {
    out << "NULL";
    return;
  }

  int number = 1;
  error_code err = 0;
  do {
    element* el;
    err = query_result_get_data(last_query_result, &el);
    if (err != 0) {
      out << "ERROR";
      return;
    }

    number = dump_element(out, el, number);
    element_dtor(el);
    free(el);

    err = query_result_next(last_query_result);
  } while (err == 0);

  query_result_dtor(last_query_result);
}

nlohmann::json StorageAdapter::outputResultAsJson() {
  if (!last_query_result || !has_result) {
    return nlohmann::json{};
  }

  auto res = result_to_json(last_query_result);

  query_result_dtor(last_query_result);

  return res;
}