cmake_minimum_required(VERSION 3.19)
project(ironjson)
set(CMAKE_CXX_STANDARD 17)

add_library(ironjson INTERFACE)
target_include_directories(ironjson INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(example1
    examples/example1.cpp
)
target_link_libraries(example1 ironjson)

add_executable(tests
    test/test.cpp
    test/test_parse.cpp
)
target_link_libraries(tests ironjson)
set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
