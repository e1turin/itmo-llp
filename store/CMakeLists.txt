cmake_minimum_required(VERSION 3.12)
project(store
        VERSION 0.0.0
        DESCRIPTION "Module implements API for low level managing \
            data in specific way (document tree & memory mapping)"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning of relative paths translation

add_library(store STATIC)
target_include_directories(store PRIVATE ${CMAKE_SOURCE_DIR})

add_subdirectory(dom)
add_subdirectory(file)
add_subdirectory(query)
