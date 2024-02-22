#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>

#include "json_to_database_query.hpp"
#include "server.hpp"
#include "storage_adapter.hpp"
#include "validate.hpp"

extern "C" {
#include "element_storage_api.h"
}

int main(int argc, char* argv[]) {
  nlohmann::json query_schema_doc;
  if (!valijson::utils::loadDocument("resources/query_schema.json", query_schema_doc)) {
    std::cerr << "Unable to open query_schema.json" << std::endl;
    return -1;
  }

  nlohmann::json element_schema_doc;
  if (!valijson::utils::loadDocument("resources/response_schema.json", element_schema_doc)) {
    std::cerr << "Unable to open response_schema.json" << std::endl;
    return -1;
  }

  StorageAdapter storage(argv[1]);
  Server server(argv[2], atoi(argv[3]));

  if (server.accept()) {
    while (true) {
      auto res = server.receive();

      if (res) {
        std::cout << "Received request: " << *res << std::endl;
      } else {
        std::cout << "Request receive error" << std::endl;
        break;
      }

      auto target_doc = nlohmann::json::parse(*res);

      nlohmann::json response_doc;

      if (!validate(query_schema_doc, target_doc)) {
        std::cerr << "Invalid json" << std::endl;
        response_doc["error"] = 1;
      } else {
        if (storage.processJsonQuery(target_doc)) {
          response_doc["error"] = 0;
          response_doc["result"] = storage.outputResultAsJson();
        } else {
          response_doc["error"] = 1;
        }
      }

      if (!server.send(response_doc.dump())) {
        std::cout << "Unable to send response" << std::endl;
        break;
      }
    }
  }

  return 0;
}