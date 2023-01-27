cmake_minimum_required(VERSION 3.21)

project(Server LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-DDEBUG")

#
# Dependencies
#
find_package(Threads)

#
# Sources
#
set(SOURCES
        src/main.cpp
        )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_subdirectory("../proto" "./proto")
include_directories("../llp/include")
#
# Target
#
add_executable(server ${SOURCES})
target_link_libraries(server
        PRIVATE
        proto
        )
target_include_directories(server
        PUBLIC "../llp/include"
        )

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
