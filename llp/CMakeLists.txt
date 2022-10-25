cmake_minimum_required(VERSION 3.23)
project(llp)

set(CMAKE_CXX_STANDARD 14)

add_library(llp STATIC src/llp.cpp include/llp.h)

include_directories(include)