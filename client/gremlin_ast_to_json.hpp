#pragma once

#include <exception>

#include <nlohmann/json.hpp>

extern "C" {
#include "gremlin_parser/ast/ast.h"
}

void gremlin_ast_to_json(AST* ast, nlohmann::json& target);