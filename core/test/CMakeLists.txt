cmake_minimum_required(VERSION 3.20.5)

link_libraries(${LITENVM_CORE_TARGET})
link_libraries(cmocka-static)

add_executable(stacktest stack_test.c)
add_test(NAME "Stack test" COMMAND stacktest)

add_executable(evalstacktest evalstack_test.c)
add_test(NAME "EvalStack test" COMMAND evalstacktest)