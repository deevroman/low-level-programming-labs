cmake_minimum_required(VERSION 3.16)
project(examples)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "-DDEBUG")


add_executable(examples main.cpp)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

target_include_directories(examples
        PUBLIC src
        PUBLIC "${CMAKE_SOURCE_DIR}/../llp/include")

#find_library(
#        LLP_LIB
#        NAMES llp libllp
#        HINTS "${CMAKE_SOURCE_DIR}/../llp/build"
#        HINTS "${CMAKE_SOURCE_DIR}/../llp/build/${CMAKE_BUILD_TYPE}"
#        NO_DEFAULT_PATH
#)

#message(STATUS "LLP_LIB: [${LLP_LIB}]")
#
#target_link_libraries(examples ${LLP_LIB})

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_${CMAKE_SYSTEM}")
