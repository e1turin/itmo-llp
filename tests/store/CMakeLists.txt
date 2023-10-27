cmake_minimum_required(VERSION 3.12)
project(tests/store)

set(LIBRARY_NAME store)
list(APPEND LIST_OF_TESTS
    store_test # -> store_test.cpp
    #[[ etc ]] # -> etc.cpp
)

SETUP_LIST_OF_TESTS(${LIBRARY_NAME} ${LIST_OF_TESTS})

#[==[
add_executable(store_test store_test.cpp)
target_link_libraries(store_test
    PRIVATE
        GTest::GTest
        store
)
target_include_directories(store_test
    PUBLIC
        ${CMAKE_SOURCE_DIR}/store/inc
)
add_test(store_gtests store_test)
]==]
