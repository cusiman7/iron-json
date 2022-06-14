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

add_executable(test
    test/test.cpp
    test/test_parse.cpp
)
target_link_libraries(test ironjson)

add_executable(bench
    bench/bench.cpp
)
target_link_libraries(bench ironjson)
target_compile_options(bench PRIVATE -fno-omit-frame-pointer)

configure_file(data/github_events.json ${CMAKE_CURRENT_BINARY_DIR}/data/github_events.json COPYONLY)
