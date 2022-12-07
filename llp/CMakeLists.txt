cmake_minimum_required(VERSION 3.16)
project(llp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-DDEBUG -fsanitize=undefined") # -Wall -pedantic -Wextra 
set(CMAKE_BUILD_TYPE Debug)

add_library(llp INTERFACE)
target_include_directories(llp INTERFACE {CMAKE_CURRENT_SOURCE_DIR})


include_directories(include)


enable_testing()
##################################
# Download and install GoogleTest

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

################

# Define a test
add_executable(simple_test tests/simple_test.cpp)
add_executable(insert_and_list tests/insert_and_list.cpp)

######################################
# Configure the test to use GoogleTest
#
# If used often, could be made a macro.

target_link_libraries(
        simple_test
        GTest::gtest_main
)
target_link_libraries(
        insert_and_list
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(simple_test)
gtest_discover_tests(insert_and_list)
