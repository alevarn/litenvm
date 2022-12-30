cmake_minimum_required(VERSION 3.20.5)

project(litenvm VERSION 1.0 LANGUAGES C)

set(LITENVM_CORE_TARGET litenvmcore)

set(LITENVM_CLI_TARGET litenvm)

add_subdirectory(core)

add_subdirectory(cli)