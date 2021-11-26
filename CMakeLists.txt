cmake_minimum_required(VERSION 2.8)
project(BuDDy)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUDDY_TARGET bdd)

# Version
set(BUDDY_VERSION_MAJOR 2)
set(BUDDY_VERSION_MINOR 4)
set(BUDDY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(src)

add_subdirectory(code)
