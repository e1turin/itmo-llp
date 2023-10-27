cmake_minimum_required(VERSION 3.12)
project(tests/store)

set(LIBRARY_NAME store)
list(APPEND LIST_OF_TESTS
    store_test # -> store_test.cpp
    #[[ etc --> etc.cpp ]]
)


foreach (TEST_NAME IN LISTS LIST_OF_TESTS)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::GTest
            ${LIBRARY_NAME}
    )
    target_include_directories(${TEST_NAME}
        PUBLIC
            ${CMAKE_SOURCE_DIR}/${LIBRARY_NAME}/inc
    )
    add_test(${LIBRARY_NAME}_gtests ${TEST_NAME})
endforeach ()

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
