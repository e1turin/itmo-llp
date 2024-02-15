cmake_minimum_required(VERSION 3.12)
project(itmo-llp
        VERSION 0.0.0
        DESCRIPTION "LLP labs"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(store)
add_subdirectory(tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(query)