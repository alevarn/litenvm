cmake_minimum_required(VERSION 3.20.5)

link_libraries(${LITENVM_CORE_TARGET})
link_libraries(cmocka-static)

add_executable(sampletest sample_test.c)
add_test(NAME sampletest COMMAND sampletest)

add_executable(sampletest2 sample2_test.c)
add_test(NAME sampletest2 COMMAND sampletest2)