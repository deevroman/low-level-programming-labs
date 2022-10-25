cmake_minimum_required(VERSION 3.16)
project(examples)

set(CMAKE_CXX_STANDARD 14)

add_executable(examples main.cpp)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_FIND_LIBRARY_PREFIXES: ${CMAKE_FIND_LIBRARY_PREFIXES}")
message(STATUS "CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")

target_include_directories(examples 
        PUBLIC src
        PUBLIC "${CMAKE_SOURCE_DIR}/../llp/include")

find_library(
        LLP_LIB
        NAMES llp libllp
        HINTS "${CMAKE_SOURCE_DIR}/../llp/build"
        NO_DEFAULT_PATH
)

message(STATUS "LLP_LIB: [${LLP_LIB}]")

target_link_libraries(examples ${LLP_LIB})

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_${CMAKE_SYSTEM}")
