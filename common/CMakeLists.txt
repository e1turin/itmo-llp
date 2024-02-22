cmake_minimum_required(VERSION 3.12)
project(common
        VERSION 0.0.0
        DESCRIPTION "Module implements API for low level managing \
            data in specific way (document tree & memory mapping)"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation


FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.3
)

FetchContent_Declare(valijson
        GIT_REPOSITORY https://github.com/tristanpenman/valijson
        GIT_TAG v1.0.2
)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(valijson)

add_library(common STATIC)
target_include_directories(common
        PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${valijson_SOURCE_DIR}/include
        ${json_SOURCE_DIR}/single_include
)
target_sources(
        unix_tcp_socket.cpp
        validate.cpp
        windows_tcp_socket.cpp
)
target_link_libraries(common json vilijson)
