cmake_minimum_required(VERSION 3.19)
project(ironjson)
set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -Wpedantic -Wextra)
if (${CMAKE_BUILD_TYPE} STREQUAL Release OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
    add_compile_options(-Werror)
endif()
#add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
#add_link_options(-fsanitize=address)

add_library(ironjson STATIC
    include/iron/json.h
    include/iron/json.cpp
)
target_include_directories(ironjson INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(examples)

add_executable(test
    test/test.cpp
    test/test_parse.cpp
    test/test_minefield.cpp
)
target_link_libraries(test ironjson)

add_executable(bench
    bench/bench.cpp
)
target_link_libraries(bench ironjson)
target_compile_options(bench PRIVATE -fno-omit-frame-pointer)

configure_file(data/demo.json ${CMAKE_CURRENT_BINARY_DIR}/data/demo.json COPYONLY)
configure_file(data/github_events.json ${CMAKE_CURRENT_BINARY_DIR}/data/github_events.json COPYONLY)
configure_file(large_data/san_fran_parcels.json ${CMAKE_CURRENT_BINARY_DIR}/large_data/san_fran_parcels.json COPYONLY)
configure_file(large_data/canada.json ${CMAKE_CURRENT_BINARY_DIR}/large_data/canada.json COPYONLY)
configure_file(large_data/twitter.json ${CMAKE_CURRENT_BINARY_DIR}/large_data/twitter.json COPYONLY)
