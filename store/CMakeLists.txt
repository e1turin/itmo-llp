cmake_minimum_required(VERSION 3.12)
project(store
        VERSION 0.0.0 # [<major>[.<minor>[.<patch>[.<tweak>]]]]
        DESCRIPTION "Module implements API for low level managing \
            data in specific way (document tree & memory mapping)"
        LANGUAGES C CXX
)

cmake_policy(SET CMP0076 NEW) # avoid warning

add_library(store STATIC)

add_subdirectory(dom)
add_subdirectory(filesystem)

## deprecated:
#target_include_directories(store PRIVATE include)
#
#target_sources(store PRIVATE
#        src/store.cpp
#        src/store2.cpp
#        #[[ etc. ]]
#)
