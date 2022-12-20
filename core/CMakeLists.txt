set(LITENVM_CORE_TARGET litenvmcore)

set(SRC_DIR src)
set(INC_DIR include)

set(SRC_FILES 
    ${SRC_DIR}/evalstack.c
    ${SRC_DIR}/config.c
)

# Build the litenvm core library. 
add_library(${LITENVM_CORE_TARGET} STATIC ${SRC_FILES})
target_include_directories(${LITENVM_CORE_TARGET} PUBLIC ${INC_DIR})

# Fetch the cmocka library and build all tests. 
include(cmake/FetchCMocka.cmake)
enable_testing()
add_subdirectory(test)