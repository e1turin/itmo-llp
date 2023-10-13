cmake_minimum_required(VERSION 3.12)
project(itmo-llp
        VERSION 0.0.0
        DESCRIPTION "LLP labs"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(store)

add_executable(main main.cpp)

target_include_directories(main PUBLIC store/inc)
target_link_libraries(main PUBLIC store)
#target_sources(main PRIVATE
#        main.cpp
#        #...
#)


