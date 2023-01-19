cmake_minimum_required(VERSION 3.21)

project(Parser LANGUAGES C)

set(SRC_DIR "src")
set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/lexer.c")
set(PARSER_OUT "${PARSER_DIR}/parser.c")

include_directories(SRC_DIR)
include_directories(CMAKE_CURRENT_BINARY_DIR)

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_executable(parser "${SRC_DIR}/main.c" "${LEXER_OUT}" "${PARSER_OUT}")
target_include_directories(parser PRIVATE "${PARSER_DIR}" "${SRC_DIR}")

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")

