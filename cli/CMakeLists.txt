cmake_minimum_required(VERSION 3.20.5)

add_executable(${LITENVM_CLI_TARGET} src/cli.c)
target_link_libraries(${LITENVM_CLI_TARGET} PRIVATE ${LITENVM_CORE_TARGET})
message("This is the version: " ${PROJECT_VERSION})
target_compile_definitions(${LITENVM_CLI_TARGET} PRIVATE LITENVM_VERSION="${PROJECT_VERSION}")