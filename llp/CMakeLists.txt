cmake_minimum_required(VERSION 3.23)
project(llp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-DDEBUG")
set(CMAKE_BUILD_TYPE Debug)

add_library(llp INTERFACE)
target_include_directories(llp INTERFACE {CMAKE_CURRENT_SOURCE_DIR})

#add_library(llp
#        include/llp.h
#        include/logger.h
#        include/query.h
#        include/storage_link.h
#        include/database.h
#        include/data_portion.h
#        include/data_item.h
#        include/file_interface.h)


include_directories(include)