cmake_minimum_required(VERSION 3.21)

project(Parser LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

set(SRC_DIR "src")
set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}/parser")
file(MAKE_DIRECTORY ${PARSER_DIR}) 

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/lexer.c")
set(PARSER_OUT "${PARSER_DIR}/parser.c")
configure_file("${SRC_DIR}/types.h" "${PARSER_DIR}/types.h" COPYONLY)

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_library(flex_bison STATIC "${LEXER_OUT}" "${PARSER_OUT}" "${SRC_DIR}/types.h")
set_target_properties(flex_bison PROPERTIES LINKER_LANGUAGE C)
target_include_directories(flex_bison PRIVATE "${SRC_DIR}")

#find_package(Protobuf REQUIRED)
#protobuf_generate_cpp(PROTO_SRC PROTO_HEADER "${CMAKE_SOURCE_DIR}/proto/message.proto")
#add_library(../proto ${PROTO_HEADER} ${PROTO_SRC})
#target_include_directories(../proto PUBLIC "${PROTOBUF_INCLUDE_DIRS}")

add_executable(parser "${SRC_DIR}/main.cpp" "${LEXER_OUT}" "${PARSER_OUT}" "${PROTO_SRC}" "${SRC_DIR}/types.h")
set_target_properties(parser PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(parser PRIVATE "${PARSER_DIR}" "${SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" "${PROTOBUF_INCLUDE_DIRS}")
target_link_libraries(parser PRIVATE flex_bison #[[../proto ${PROTOBUF_LIBRARY}]])

set(EXECUTABLE_OUTPUT_PATH "./bin/${CMAKE_SYSTEM}")
