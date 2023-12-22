cmake_minimum_required(VERSION 3.12)
project(tests/store)

list(APPEND LIST_OF_TESTS
        store_test
        store2_test
)

SETUP_LIST_OF_TESTS(store "${LIST_OF_TESTS}")
