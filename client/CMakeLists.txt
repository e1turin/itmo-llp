cmake_minimum_required(VERSION 3.12)
project(client
        VERSION 0.0.0
        DESCRIPTION "Module implements API for low level managing \
            data in specific way (document tree & memory mapping)"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_library(client STATIC)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(client common query)

target_sources(client
        PRIVATE
        client.cpp
        gremlin_ast_to_json.cpp
        json_to_response.cpp
        response.cpp
)

add_executable(client_main)
target_include_directories(client_main PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(client_main main.cpp)
target_link_libraries(client_main client)
