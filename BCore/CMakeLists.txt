# TODO 1: Set the minimum required version of CMake to be 3.10
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0007 NEW)

include(${CMAKE_CURRENT_SOURCE_DIR}/../Utils.cmake)

project(BCore 
VERSION 1.0
DESCRIPTION "Core library providing Allocators , Containers , custom String ,etc ..." )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

message("-- Path infos")
message("\t --Project path : ${PROJECT_SOURCE_DIR}")
message("\t --CMake path : ${CMAKE_SOURCE_DIR}")

# path to build files
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/Build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH})

SET(BUILD_MODE Debug)
add_compile_definitions("_CORE_EXPORT")
add_compile_definitions("_DEBUG")
add_compile_options("/Zc:preprocessor")
# enable exporting all symbols , important for exporting dllexport members/classes
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# get list of all source files
set(sources "")
read_source_files("CMakeSources.txt" sources)
log_source_files(sources)

# get list of all the header files
FILE(GLOB_RECURSE headers "*.h")
log_header_files(headers)

message("-- Build path")
message("\t-- ${BUILD_PATH}")
message("\r")

add_library(BCoreDLL SHARED ${sources} ${headers})
set_property(TARGET BCoreDLL PROPERTY OUTPUT_NAME BCore)
target_include_directories(BCoreDLL PUBLIC  ${PROJECT_BINARY_DIR})

# TODO 9: Use target_include_directories to include ${PROJECT_BINARY_DIR}