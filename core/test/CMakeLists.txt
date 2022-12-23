cmake_minimum_required(VERSION 3.20.5)

link_libraries(${LITENVM_CORE_TARGET})
link_libraries(cmocka-static)

add_executable(stacktest stack_test.c)
add_test(NAME "Stack test" COMMAND stacktest)

add_executable(evalstacktest evalstack_test.c)
add_test(NAME "EvalStack test" COMMAND evalstacktest)

add_executable(callstacktest callstack_test.c)
add_test(NAME "CallStack test" COMMAND callstacktest)

add_executable(constantpooltest constantpool_test.c)
add_test(NAME "ConstantPool test" COMMAND constantpooltest)

add_executable(executortest executor_test.c)
add_test(NAME "Executor test" COMMAND executortest)