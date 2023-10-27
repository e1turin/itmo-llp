cmake_minimum_required(VERSION 3.12)
project(tests)

## [FROM GOOGLE TEST DOCS[
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
## ]FROM GOOGLE TEST DOCS]

add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)


function(SETUP_LIST_OF_TESTS LIBRARY_NAME LIST_OF_TESTS)
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
endfunction()

add_subdirectory(store)
#[[ add_subdirectory(...) ]]
