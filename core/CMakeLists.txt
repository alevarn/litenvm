set(LITENVM_CORE_TARGET litenvmcore)

set(SRC_DIR src)
set(SRC_CLS_DIR ${SRC_DIR}/classes)
set(INC_DIR include)
set(INC_CLS_DIR ${INC_DIR}/classes)

set(SRC_FILES 
    ${SRC_DIR}/config.c
    ${SRC_DIR}/stack.c
    ${SRC_DIR}/evalstack.c
    ${SRC_DIR}/callstack.c
    ${SRC_DIR}/constantpool.c
    ${SRC_DIR}/inststream.c
    ${SRC_DIR}/vtable.c
    ${SRC_DIR}/object.c
    ${SRC_CLS_DIR}/string_class.c 
    ${SRC_DIR}/executor.c
)

# Build the litenvm core library. 
add_library(${LITENVM_CORE_TARGET} STATIC ${SRC_FILES})
target_include_directories(${LITENVM_CORE_TARGET} PUBLIC ${INC_DIR})
target_include_directories(${LITENVM_CORE_TARGET} PUBLIC ${INC_CLS_DIR})
target_link_libraries(${LITENVM_CORE_TARGET} m)

# Fetch the cmocka library and build all tests. 
include(cmake/FetchCMocka.cmake)
enable_testing()
add_subdirectory(test)