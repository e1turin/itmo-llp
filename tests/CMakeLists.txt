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

include(GoogleTest)

#[[
Function for quick setup tests for every needed module.
- `lib-name` is module name I'm testing.
- `list-of-tests` is list which contains names of test files
  without `.cpp` extension.
NOTE: Don't forget to wrap variable of list with quotation
      marks to pass it list.
]]
function(SETUP_LIST_OF_TESTS lib-name list-of-tests)
    foreach (test-name ${list-of-tests})
        add_executable(${test-name}_gtests ${test-name}.cpp)
        target_link_libraries(${test-name}_gtests
            PRIVATE
                GTest::GTest
                ${lib-name}
        )
        target_include_directories(${test-name}_gtests
            PUBLIC
                ${CMAKE_SOURCE_DIR}/${lib-name}/include
        )
        gtest_discover_tests(${test-name}_gtests)
    endforeach ()
endfunction()

add_subdirectory(store)
#[[ add_subdirectory(...) ]]
