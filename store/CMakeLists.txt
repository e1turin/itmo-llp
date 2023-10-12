cmake_minimum_required(VERSION 3.12)
project(store)

add_executable(store
        main.cpp
        VERSION 0.0.0 # [<major>[.<minor>[.<patch>[.<tweak>]]]]
        DESCRIPTION Module implements API for low level managing
        data in specific way (document tree & memory mapping)
        #        [HOMEPAGE_URL <url-string>]
        #        [LANGUAGES <language-name>...]
)