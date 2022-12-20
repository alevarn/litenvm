cmake_minimum_required(VERSION 3.20.5)

link_libraries(${LITENVM_CORE_TARGET})
link_libraries(cmocka-static)

add_executable(evalstacktest evalstack_test.c)
add_test(NAME "Evaluation stack tests" COMMAND evalstacktest)