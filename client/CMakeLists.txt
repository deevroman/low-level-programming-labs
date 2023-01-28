cmake_minimum_required(VERSION 3.21)

project(Client LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads)

set(SOURCES
        src/main.cpp
        )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_subdirectory("../proto" "./proto")
add_subdirectory("../parser" "./parser")

add_executable(client ${SOURCES})
target_link_libraries(client
        PRIVATE
        proto
        flex_bison
        )

target_include_directories(client PRIVATE "${PARSER_DIR}" "${PARSER_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/parser/parser" "${PROTOBUF_INCLUDE_DIRS}")

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
