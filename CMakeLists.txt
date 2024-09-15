# TODO 1: Set the minimum required version of CMake to be 3.10
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(SET CMP0007 NEW)

project(BEngine 
VERSION 1.0
DESCRIPTION "Engine & Client" )
# TODO 7: Set the project version number as 1.0 in the above project command

# TODO 6: Set the variable CMAKE_CXX_STANDARD to 11
#         and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(BCore "/BCore")
set(BEngine "/BEngine")
set(CustomGame "/CustomGame")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${BCore})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${BEngine})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${CustomGame})

# TODO 9: Use target_include_directories to include ${PROJECT_BINARY_DIR}
# target_include_directories(ExecutableTarget PUBLIC "${PROJECT_BINARY_DIR}")
# add_library(BCore STATIC ${sources})
