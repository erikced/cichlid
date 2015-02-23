project(cichlid)

#find_package(PkgConfig)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

if(CMAKE_C_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
        OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wconversion")
endif()

add_subdirectory(src) 

