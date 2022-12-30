set(LITENVM_CORE_TARGET litenvmcore)

set(SRC_DIR src)
set(INC_DIR include)

set(SRC_FILES 
    ${SRC_DIR}/config.c
    ${SRC_DIR}/stack.c
    ${SRC_DIR}/evalstack.c
    ${SRC_DIR}/callstack.c
    ${SRC_DIR}/constantpool.c
    ${SRC_DIR}/inststream.c
    ${SRC_DIR}/vtable.c
    ${SRC_DIR}/object.c
    ${SRC_DIR}/string_class.c 
    ${SRC_DIR}/string_builder_class.c 
    ${SRC_DIR}/binary_format.c 
    ${SRC_DIR}/executor.c
)

# Build the litenvm core library. 
add_library(${LITENVM_CORE_TARGET} STATIC ${SRC_FILES})
target_include_directories(${LITENVM_CORE_TARGET} PUBLIC ${INC_DIR})
target_link_libraries(${LITENVM_CORE_TARGET} m)

# Fetch the cmocka library and build all tests. 
include(cmake/FetchCMocka.cmake)
enable_testing()
add_subdirectory(test)