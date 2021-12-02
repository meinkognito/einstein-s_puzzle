cmake_minimum_required(VERSION 2.8)

set(TARGET main)

set(SOURCES ${TARGET}.cxx)

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET} PRIVATE ${BUDDY_ROOT_DIR}/lib/headers)
target_link_libraries(${TARGET} ${BUDDY_TARGET})
