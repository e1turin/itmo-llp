cmake_minimum_required(VERSION 3.12)
project(itmo-llp
        VERSION 0.0.0 # [<major>[.<minor>[.<patch>[.<tweak>]]]]
        DESCRIPTION "LLP labs"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


enable_testing()

add_subdirectory(store)
add_subdirectory(tests)

add_executable(main)
target_sources(main
    PRIVATE
        main.cpp
        #[[ etc. ]]
)

target_include_directories(main
    PUBLIC
        store/include
        #[[ etc./include ]]
)

target_link_libraries(main
    PUBLIC
        store
        #[[ etc ]]
)
