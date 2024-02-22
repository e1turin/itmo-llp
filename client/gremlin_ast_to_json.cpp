#include "gremlin_ast_to_json.hpp"

nlohmann::json* get_next_attributes_pattern(nlohmann::json& target) {
  auto [attrs_it, success] = target.emplace("attributes", nlohmann::json{});

  if (success) {
    return &(*attrs_it);
  } else {
    auto tmp = nlohmann::json{};
    tmp["patternType"] = "BIN_OP";
    tmp["operation"] = "AND";
    tmp["lhs"] = std::move(*attrs_it);
    *attrs_it = std::move(tmp);
    return &((*attrs_it)["rhs"]);
  }
}

void gremlin_ast_to_json(AST* ast, nlohmann::json& target) {
  if (!ast) {
    return;
  }

  switch (ast->type) {
    case AST_NODE_SCHEMA_ADD_E:
      target["queryType"] = "create_edge_type";
      gremlin_ast_to_json(ast->args.as_schema_add_e.label, target["linkType"]);
      break;
    case AST_NODE_SCHEMA_DROP_E:
      target["queryType"] = "drop_edge_type";
      gremlin_ast_to_json(ast->args.as_schema_drop_e.label, target["linkType"]);
      break;
    case AST_NODE_SCHEMA_DROP_V:
      target["queryType"] = "drop_vertex_type";
      gremlin_ast_to_json(ast->args.as_schema_drop_v.label, target["elementType"]);
      break;
    case AST_NODE_SCHEMA_ADD_V:
      target["queryType"] = "create_vertex_type";
      target["attributes"] = nlohmann::json::array();
      gremlin_ast_to_json(ast->args.as_schema_add_v.label, target["elementType"]);
      gremlin_ast_to_json(ast->args.as_schema_add_v.prop_types, target);
      break;
    case AST_NODE_PROPERTY_TYPES_LIST: {
      auto attrs_it = target.find("attributes");
      switch (ast->args.as_prop_types_list.prop_type) {
        case AST_NODE_PROPERTY_PAIR:
          attrs_it->emplace_back(nlohmann::json{});
          gremlin_ast_to_json(ast->args.as_prop_types_list.value.as_pair.lhs, attrs_it->back());
          gremlin_ast_to_json(ast->args.as_prop_types_list.value.as_pair.rhs, target);
          break;
        case AST_NODE_PROPERTY_BOOL:
          target = "BOOL";
          break;
        case AST_NODE_PROPERTY_DOUBLE:
          target = "DOUBLE";
          break;
        case AST_NODE_PROPERTY_INT32:
          target = "INT32";
          break;
        case AST_NODE_PROPERTY_INT64:
          target = "INT64";
          break;
        case AST_NODE_PROPERTY_STRING:
          target = "STRING";
          break;
        default:
          break;
      }
      break;
    }
    case AST_NODE_GRAPH_ADD_V: {
      target["queryType"] = "create_vertex";
      auto [element_it, success] = target.emplace("element", nlohmann::json{});

      gremlin_ast_to_json(ast->args.as_graph_add_v.label, (*element_it)["elementType"]);
      gremlin_ast_to_json(ast->args.as_graph_add_v.properties, *element_it);
      break;
    }
    case AST_NODE_PROPERTY_VALUES_LIST: {
      auto [attrs_it, success] = target.emplace("attributes", nlohmann::json::array());

      attrs_it->emplace_back(nlohmann::json{});
      gremlin_ast_to_json(ast->args.as_prop_values_list.lhs, attrs_it->back());
      gremlin_ast_to_json(ast->args.as_prop_values_list.rhs, target);
      break;
    }
    case AST_NODE_GRAPH_UPDATE:
      target["queryType"] = "update_vertex";
      gremlin_ast_to_json(ast->args.as_graph_update.specific_v, target["pattern"]);
      gremlin_ast_to_json(ast->args.as_graph_update.prop_updates_list, target);
      break;
    case AST_NODE_PROPERTY_UPDATES_LIST: {
      auto [attrs_it, success] = target.emplace("attributes", nlohmann::json::array());

      attrs_it->emplace_back(nlohmann::json{});
      gremlin_ast_to_json(ast->args.as_prop_update_list.lhs, attrs_it->back());
      gremlin_ast_to_json(ast->args.as_prop_update_list.rhs, target);
      break;
    }
    case AST_NODE_PROPERTY_UPDATE:
      gremlin_ast_to_json(ast->args.as_prop_update.property_id, target);
      gremlin_ast_to_json(ast->args.as_prop_update.value, target["value"]);
      break;
    case AST_NODE_GRAPH_SELECT:
      target["queryType"] = "match";
      gremlin_ast_to_json(ast->args.as_select.specific_v, target["pattern"]);
      break;
    case AST_NODE_GRAPH_DROP_V:
      target["queryType"] = "drop_vertex";
      gremlin_ast_to_json(ast->args.as_graph_drop_v.specific_v, target["pattern"]);
      break;
    case AST_NODE_GRAPH_ADD_E:
      target["queryType"] = "create_edge";
      gremlin_ast_to_json(ast->args.as_graph_add_e.label, target["linkType"]);
      gremlin_ast_to_json(ast->args.as_graph_add_e.src_specific_v, target["sourcePattern"]);
      gremlin_ast_to_json(ast->args.as_graph_add_e.dst_specific_v, target["dstPattern"]);
      break;
    case AST_NODE_LIT:
      switch (ast->args.as_literal.type) {
        case AST_NODE_LIT_DOUBLE:
          target = ast->args.as_literal.value.as_double;
          break;
        case AST_NODE_LIT_INT:
          target = ast->args.as_literal.value.as_int;
          break;
        case AST_NODE_LIT_STRING:
          target = ast->args.as_literal.value.as_string;
          ast->args.as_literal.value.as_string = nullptr;
          break;
        default:
          throw std::logic_error("Unknown literal type!");
          break;
      }
      break;
    case AST_NODE_SPECIFIC_V:
      gremlin_ast_to_json(ast->args.as_specific_v.label, target["elementType"]);
      target["linksIn"] = nlohmann::json::array();
      target["linksOut"] = nlohmann::json::array();
      gremlin_ast_to_json(ast->args.as_specific_v.conditions, target);
      break;
    case AST_NODE_V_CONDITIONS:
      switch (ast->args.as_conditions.cond_type) {
        case AST_NODE_V_COND_PAIR: {
          auto lhs = ast->args.as_conditions.value.as_pair.lhs;

          if (lhs->type != AST_NODE_V_CONDITIONS) {
            throw std::logic_error(
                "Invalid ast, condition pair must contain pair of "
                "AST_NODE_V_CONDITIONS nodes");
          }

          if (lhs->args.as_conditions.cond_type == AST_NODE_V_COND_IN) {
            auto links_in_it = target.find("linksIn");
            links_in_it->emplace_back(nlohmann::json{});

            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.lhs, links_in_it->back());
            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.rhs, links_in_it->back()["target"]);
          } else if (lhs->args.as_conditions.cond_type == AST_NODE_V_COND_OUT) {
            auto links_out_it = target.find("linksOut");
            links_out_it->emplace_back(nlohmann::json{});

            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.lhs, links_out_it->back());
            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.rhs, links_out_it->back()["target"]);
          } else {
            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.lhs, target);
            gremlin_ast_to_json(ast->args.as_conditions.value.as_pair.rhs, target);
          }

          break;
        }
        case AST_NODE_V_COND_IN:
          gremlin_ast_to_json(ast->args.as_conditions.value.as_in.label, target["linkType"]);
          break;
        case AST_NODE_V_COND_OUT:
          gremlin_ast_to_json(ast->args.as_conditions.value.as_out.label, target["linkType"]);
          break;
        case AST_NODE_V_COND_HAS: {
          nlohmann::json* next_attr_pattern = get_next_attributes_pattern(target);

          (*next_attr_pattern)["patternType"] = "COND";
          gremlin_ast_to_json(ast->args.as_conditions.value.as_has.prop_id, *next_attr_pattern);
          gremlin_ast_to_json(ast->args.as_conditions.value.as_has.value_cond, *next_attr_pattern);
          break;
        }
        case AST_NODE_V_COND_OR: {
          nlohmann::json* next_attr_pattern = get_next_attributes_pattern(target);

          (*next_attr_pattern)["patternType"] = "BIN_OP";
          (*next_attr_pattern)["operation"] = "OR";

          gremlin_ast_to_json(ast->args.as_conditions.value.as_or.lhs, (*next_attr_pattern)["lhs"]);
          gremlin_ast_to_json(ast->args.as_conditions.value.as_or.rhs, (*next_attr_pattern)["rhs"]);
          break;
        }
        case AST_NODE_V_COND_FILTER:
          gremlin_ast_to_json(ast->args.as_conditions.value.as_filter.conditions, target);
          break;
        default:
          throw std::logic_error("Unknown ast condition node");
          break;
      }
      break;
    case AST_NODE_PROPERTY_ID:
      gremlin_ast_to_json(ast->args.as_property_id.id, target["index"]);
      break;
    case AST_NODE_VALUE_COND:
      switch (ast->args.as_value_cond.type) {
        case AST_NODE_VALUE_COND_EQ:
          target["operation"] = "EQ";
          break;
        case AST_NODE_VALUE_COND_GT:
          target["operation"] = "GT";
          break;
        case AST_NODE_VALUE_COND_GTE:
          target["operation"] = "GTE";
          break;
        case AST_NODE_VALUE_COND_LT:
          target["operation"] = "LT";
          break;
        case AST_NODE_VALUE_COND_LTE:
          target["operation"] = "LTE";
          break;
        case AST_NODE_VALUE_COND_NEQ:
          target["operation"] = "NEQ";
          break;
        case AST_NODE_VALUE_COND_CONTAINS:
          target["operation"] = "CONTAINS";
          break;
        default:
          throw std::logic_error("Unknown value condition node type");
          break;
      }
      gremlin_ast_to_json(ast->args.as_value_cond.value, target["value"]);
      break;
    default:
      throw std::logic_error("Unsupported type of ast node " + std::to_string(ast->type));
      break;
  }
}