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


add_subdirectory(store)

# Now simply link against gtest or gtest_main as needed. Eg
#[[
add_executable(store_test store_test.cpp)
target_link_libraries(store_test PRIVATE GTest::GTest store)
add_test(store_gtests store_test)
]]
