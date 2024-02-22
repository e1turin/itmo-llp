#include "json_to_database_query.hpp"

#include <cstdlib>
#include <stdexcept>

void fill_string(const nlohmann::json& source, char** target) {
  const auto& str = source.get<std::string>();
  *target = (char*)calloc(1, str.size() + 1);
  std::memcpy(*target, str.c_str(), str.size() + 1);
}

void fill_attr(const nlohmann::json& source, attribute& target) {
  switch (source.type()) {
    case nlohmann::detail::value_t::number_integer: {
      target.type = ATTR_TYPE_INT64;
      target.value.as_int64 = source.get<int64_t>();
      break;
    }
    case nlohmann::detail::value_t::number_unsigned: {
      target.type = ATTR_TYPE_INT64;
      target.value.as_int64 = source.get<int64_t>();
      break;
    }
    case nlohmann::detail::value_t::number_float: {
      target.type = ATTR_TYPE_DOUBLE;
      target.value.as_double = source.get<double>();
      break;
    }
    case nlohmann::detail::value_t::string: {
      target.type = ATTR_TYPE_STRING;
      fill_string(source, &(target.value.as_string.data));
      break;
    }
    case nlohmann::detail::value_t::boolean: {
      target.type = ATTR_TYPE_BOOL;
      target.value.as_bool = source.get<bool>();
      break;
    }
    default:
      throw std::logic_error("Unknown attr type: " + source.dump());
      break;
  }
}

void fill_attr_pattern(const nlohmann::json& source, attribute_pattern** pattern) {
  *pattern = (attribute_pattern*)calloc(1, sizeof(attribute_pattern));

  const auto& pattern_type_str = source["patternType"];
  if (pattern_type_str == "BIN_OP") {
    (*pattern)->node_type = ATTRIBUTE_PATTERN_BIN_OP;

    if (source["operation"] == "AND") {
      (*pattern)->args.as_bin.op_type = ATTRIBUTE_PATTERN_AND;
    } else {
      (*pattern)->args.as_bin.op_type = ATTRIBUTE_PATTERN_OR;
    }

    fill_attr_pattern(source["lhs"], &((*pattern)->args.as_bin.lhs));
    fill_attr_pattern(source["rhs"], &((*pattern)->args.as_bin.rhs));
  } else {
    const auto& op = source["operation"].get<std::string>();

    if (op == "EQ" || op == "LT" || op == "GT" || op == "CONTAINS") {
      (*pattern)->node_type = ATTRIBUTE_PATTERN_COND;

      if (op == "EQ") {
        (*pattern)->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_EQ;
      } else if (op == "LT") {
        (*pattern)->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_LESS;
      } else if (op == "GT") {
        (*pattern)->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_GREATER;
      } else {
        (*pattern)->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_CONTAINS;
      }

      (*pattern)->args.as_cond.attr_id = source["index"];
      fill_attr(source["value"], (*pattern)->args.as_cond.attr);
    } else if (op == "LTE" || op == "GTE") {
      (*pattern)->node_type = ATTRIBUTE_PATTERN_BIN_OP;
      (*pattern)->args.as_bin.op_type = ATTRIBUTE_PATTERN_OR;

      (*pattern)->args.as_bin.lhs = (attribute_pattern*)calloc(1, sizeof(attribute_pattern));
      (*pattern)->args.as_bin.rhs = (attribute_pattern*)calloc(1, sizeof(attribute_pattern));

      auto& lhs = (*pattern)->args.as_bin.lhs;
      auto& rhs = (*pattern)->args.as_bin.rhs;

      lhs->node_type = ATTRIBUTE_PATTERN_COND;
      rhs->node_type = ATTRIBUTE_PATTERN_COND;

      lhs->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_EQ;
      if (op == "LTE") {
        rhs->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_LESS;
      } else {
        rhs->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_GREATER;
      }

      auto val_it = source.find("value");
      fill_attr(*val_it, lhs->args.as_cond.attr);
      fill_attr(*val_it, rhs->args.as_cond.attr);

      int index = source["index"];
      lhs->args.as_cond.attr_id = index;
      rhs->args.as_cond.attr_id = index;
    } else if (op == "NEQ") {
      (*pattern)->node_type = ATTRIBUTE_PATTERN_UN_OP;
      (*pattern)->args.as_unary.op_type = ATTRIBUTE_PATTERN_NOT;
      (*pattern)->args.as_unary.arg = (attribute_pattern*)calloc(1, sizeof(attribute_pattern));
      (*pattern)->args.as_unary.arg->node_type = ATTRIBUTE_PATTERN_COND;
      (*pattern)->args.as_unary.arg->args.as_cond.attr_id = source["index"];
      (*pattern)->args.as_unary.arg->args.as_cond.cond = ATTRIBUTE_PATTERN_COND_EQ;
      fill_attr(source["value"], (*pattern)->args.as_unary.arg->args.as_cond.attr);
    } else {
      throw std::runtime_error("Unsupported operation in attribute pattern");
    }
  }
}

void fill_pattern(const nlohmann::json& source, element_pattern& target);

void fill_link_pattern(const nlohmann::json& source, link_pattern& pattern) {
  pattern.link_type = (link_type*)calloc(1, sizeof(link_type));
  fill_string(source["linkType"], &(pattern.link_type->type_name));

  pattern.target = (element_pattern*)calloc(1, sizeof(element_pattern));
  fill_pattern(source["target"], *(pattern.target));
}

void fill_pattern(const nlohmann::json& source, element_pattern& target) {
  target.element_type = (element_type*)calloc(1, sizeof(element_type));
  fill_string(source["elementType"], &(target.element_type->type_name));

  auto attr_pattern_it = source.find("attributes");
  if (attr_pattern_it != source.end()) {
    fill_attr_pattern(*attr_pattern_it, &(target.attr_pattern));
  }

  auto links_in_it = source.find("linksIn");
  auto links_out_it = source.find("linksOut");

  target.links_in_cnt = links_in_it->size();
  target.links_out_cnt = links_out_it->size();

  if (links_in_it->size() != 0) {
    target.links_in = (link_pattern*)calloc(links_in_it->size(), sizeof(link_pattern));
  }
  if (links_out_it->size() != 0) {
    target.links_out = (link_pattern*)calloc(links_out_it->size(), sizeof(link_pattern));
  }

  for (size_t i = 0; i < links_in_it->size(); ++i) {
    fill_link_pattern((*links_in_it)[i], target.links_in[i]);
  }
  for (size_t i = 0; i < links_out_it->size(); ++i) {
    fill_link_pattern((*links_out_it)[i], target.links_out[i]);
  }
}

void json_to_database_query(const nlohmann::json& source, query& target) {
  auto query_type_it = source.find("queryType");
  if (query_type_it == source.end()) {
    throw std::logic_error("Invalid json, expected to have queryType field");
  }

  auto query_type = query_type_it->get<std::string>();
  if (query_type == "create_vertex") {
    target.type = QUERY_TYPE_CREATE;

    auto element_it = source.find("element");

    target.args.as_create.element = (element*)calloc(1, sizeof(element));
    target.args.as_create.element->element_type = (element_type*)calloc(1, sizeof(element_type));

    fill_string((*element_it)["elementType"], &(target.args.as_create.element->element_type->type_name));

    auto attrs_it = element_it->find("attributes");

    target.args.as_create.element->attribute_count = attrs_it->size();
    target.args.as_create.element->attributes = (attribute*)calloc(attrs_it->size(), sizeof(attribute));

    for (size_t i = 0; i < attrs_it->size(); ++i) {
      fill_attr((*attrs_it)[i], target.args.as_create.element->attributes[i]);
    }
  }

  if (query_type == "drop_vertex") {
    target.type = QUERY_TYPE_DELETE;
    target.args.as_delete.pattern = (element_pattern*)calloc(1, sizeof(element_pattern));
    fill_pattern(source["pattern"], *target.args.as_delete.pattern);
  }
  if (query_type == "match") {
    target.type = QUERY_TYPE_MATCH;
    target.args.as_match.pattern = (element_pattern*)calloc(1, sizeof(element_pattern));
    fill_pattern(source["pattern"], *target.args.as_match.pattern);
  }
  if (query_type == "create_edge") {
    target.type = QUERY_TYPE_LINK;
    target.args.as_link.source_pattern = (element_pattern*)calloc(1, sizeof(element_pattern));
    fill_pattern(source["sourcePattern"], *target.args.as_link.source_pattern);

    target.args.as_link.dst_pattern = (element_pattern*)calloc(1, sizeof(element_pattern));
    fill_pattern(source["dstPattern"], *target.args.as_link.dst_pattern);

    target.args.as_link.link_type = (link_type*)calloc(1, sizeof(link_type));
    fill_string(source["linkType"], &(target.args.as_link.link_type->type_name));
  }
  if (query_type == "update_vertex") {
    target.type = QUERY_TYPE_SET;

    target.args.as_set.pattern = (element_pattern*)calloc(1, sizeof(element_pattern));
    fill_pattern(source["pattern"], *target.args.as_set.pattern);

    auto attrs_it = source.find("attributes");
    target.args.as_set.attributes_count = attrs_it->size();
    target.args.as_set.attributes =
        (set_args::attribute_and_id*)calloc(attrs_it->size(), sizeof(set_args::attribute_and_id));

    for (size_t i = 0; i < attrs_it->size(); ++i) {
      target.args.as_set.attributes[i].attribute_id = (*attrs_it)[i]["index"];
      fill_attr((*attrs_it)[i]["value"], target.args.as_set.attributes[i].attribute);
    }
  }
  if (query_type == "create_vertex_type") {
    target.type = QUERY_TYPE_CREATE_VERTEX_TYPE;

    target.args.as_create_vertex_type.type = (element_type*)calloc(1, sizeof(element_type));

    auto& type = target.args.as_create_vertex_type.type;

    fill_string(source["elementType"], &(type->type_name));

    auto attrs_it = source.find("attributes");
    type->attribute_count = attrs_it->size();

    type->attribute_types = (ATTRIBUTE_TYPE*)calloc(attrs_it->size(), sizeof(ATTRIBUTE_TYPE));
    for (size_t i = 0; i < attrs_it->size(); ++i) {
      const auto& type_name = (*attrs_it)[i].get<std::string>();
      if (type_name == "INT32") {
        type->attribute_types[i] = ATTR_TYPE_INT32;
      } else if (type_name == "INT64") {
        type->attribute_types[i] = ATTR_TYPE_INT64;
      } else if (type_name == "DOUBLE") {
        type->attribute_types[i] = ATTR_TYPE_DOUBLE;
      } else if (type_name == "BOOL") {
        type->attribute_types[i] = ATTR_TYPE_BOOL;
      } else if (type_name == "STRING") {
        type->attribute_types[i] = ATTR_TYPE_STRING;
      } else {
        throw std::logic_error("Unknown name of type");
      }
    }
  }
  if (query_type == "create_edge_type") {
    target.type = QUERY_TYPE_CREATE_EDGE_TYPE;

    target.args.as_create_edge_type.type = (link_type*)calloc(1, sizeof(link_type));
    fill_string(source["linkType"], &(target.args.as_create_edge_type.type->type_name));
  }
  if (query_type == "drop_edge_type") {
    target.type = QUERY_TYPE_DROP_EDGE_TYPE;
    fill_string(source["linkType"], &(target.args.as_drop_edge_type.type_name));
  }
  if (query_type == "drop_vertex_type") {
    target.type = QUERY_TYPE_DROP_VERTEX_TYPE;
    fill_string(source["elementType"], &(target.args.as_drop_vertex_type.type_name));
  }
}