#include <iostream>

#include <nlohmann/json.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>

#include "client.hpp"
#include "gremlin_ast_to_json.hpp"
#include "json_to_response.hpp"
#include "response.hpp"
#include "validate.hpp"

extern "C" {
#include "gremlin_parser/api/ast_api.h"
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Wrong arguments count! Expected 3 arguments: ip, port, schema" << std::endl;
    return -1;
  }

  nlohmann::json schema_doc;
  if (!valijson::utils::loadDocument(argv[3], schema_doc)) {
    std::cerr << "Unable to open " << argv[3] << std::endl;
    return -1;
  }

  Client client;
  if (!client.connect(argv[1], atoi(argv[2]))) {
    std::cerr << "Unable to connect to " << argv[1] << ":" << argv[2] << std::endl;
    return -1;
  }

  while (true) {
    AST* ast = ast_parse();
    if (!ast) {
      std::cerr << "Error occurred during parsing of gremlin query" << std::endl;
      continue;
    }

    nlohmann::json target_doc;
    gremlin_ast_to_json(ast, target_doc);
    ast_free(ast);

    if (!validate(schema_doc, target_doc)) {
      std::cerr << "Invalid json" << std::endl;
      continue;
    }

    if (!client.send(target_doc.dump())) {
      std::cerr << "Unable to send" << std::endl;
      return -1;
    }

    auto res = client.receive();

    if (!res) {
      std::cerr << "Unable to receive response" << std::endl;
      return -1;
    }

    auto response_doc = nlohmann::json::parse(*res);
    Response response = json_to_response(response_doc);

    response.dump(std::cout);
  }

  return 0;
}
