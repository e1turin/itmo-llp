cmake_minimum_required(VERSION 3.12)
project(store
        VERSION 0.0.0
        DESCRIPTION "Module implements parser logic for specified query language"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

target_include_directories(store PRIVATE ${CMAKE_SOURCE_DIR})

target_sources(query
        PRIVATE
        query.cpp
)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}/parser")
file(MAKE_DIRECTORY ${PARSER_DIR})

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/lexer.c")
set(PARSER_OUT "${PARSER_DIR}/parser.c")
configure_file("${SRC_DIR}/types.h" "${PARSER_DIR}/types.h" COPYONLY)

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_library(flex_bison STATIC "${LEXER_OUT}" "${PARSER_OUT}" "${SRC_DIR}/types.h")
set_target_properties(flex_bison PROPERTIES LINKER_LANGUAGE C)
target_include_directories(flex_bison PRIVATE "${SRC_DIR}")

add_executable(parser "${SRC_DIR}/main.c" "${LEXER_OUT}" "${PARSER_OUT}" "${PROTO_SRC}" "${SRC_DIR}/types.h")
target_include_directories(parser PRIVATE "${PARSER_DIR}" "${SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "${PROTOBUF_INCLUDE_DIRS}")
target_link_libraries(parser PRIVATE flex_bison)

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")