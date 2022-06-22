
function(add_example NAME)
    add_executable(${NAME}
        ${ARGN}
    )
    target_link_libraries(${NAME} ironjson)
endfunction()

add_example(compile_size compile_size.cpp)
add_example(create create.cpp)
add_example(example1 example1.cpp)
add_example(round_trip round_trip.cpp)
