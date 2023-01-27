cmake_minimum_required(VERSION 3.21)

project(Client LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
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
add_subdirectory("../parser" "./parser")

#
# Target
#
add_executable(client ${SOURCES})
target_link_libraries(client
        PRIVATE
        proto
        flex_bison
        )



#set(CUR_DIR "${CMAKE_SOURCE_DIR}")
#set(CMAKE_SOURCE_DIR "../parser")
#include("../parser/CMakeLists.txt")
#
#add_executable(client "src/main.cpp" "${CUR_DIR}/../parser/src/types.h")
#set_target_properties(client PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(client PRIVATE "${PARSER_DIR}" "${PARSER_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/parser/parser" "${PROTOBUF_INCLUDE_DIRS}")
#target_link_libraries(client PRIVATE flex_bison proto ${PROTOBUF_LIBRARY})

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
