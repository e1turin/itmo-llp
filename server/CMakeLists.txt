cmake_minimum_required(VERSION 3.12)
project(server
        VERSION 0.0.0
        DESCRIPTION "Module implements API for low level managing \
            data in specific way (document tree & memory mapping)"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_library(server STATIC)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(server common server store)

target_sources(server
        PRIVATE
        server.cpp
        json_to_database_query.cpp
        result_to_json.cpp
        storage_adapter.cpp
)

add_executable(server_main)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(server_main main.cpp)
target_link_libraries(server_main server)


