cmake_minimum_required(VERSION 3.12)
project(itmo-llp
        VERSION 0.0.0 # [<major>[.<minor>[.<patch>[.<tweak>]]]]
        DESCRIPTION "LLP labs"
        #        [HOMEPAGE_URL <url-string>]
        LANGUAGES CXX
)

#include_directories(store)

add_executable(main
    main.cpp
)