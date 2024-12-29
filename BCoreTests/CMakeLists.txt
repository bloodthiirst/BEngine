cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0007 NEW)

include(${CMAKE_CURRENT_SOURCE_DIR}/../Utils.cmake)

project(BCoreTests
VERSION 1.0
DESCRIPTION "Test for BCore" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

message("-- Path infos")
message("\t --Project path : ${PROJECT_SOURCE_DIR}")
message("\t --CMake path : ${CMAKE_SOURCE_DIR}")

# path to build files
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/Build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH})

set(BUILD_MODE Debug)
add_compile_options("/Zc:preprocessor")
add_compile_options("/fsanitize=address")
add_compile_options("/INFERASANLIBS")
add_compile_options("/MP")

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

# add BCore headers
cmake_path(GET PROJECT_BINARY_DIR PARENT_PATH parentPath)
cmake_path(GET parentPath PARENT_PATH parentPath)

set(BCoreHeaders "${parentPath}/BCore")
message("\t-- Adding BCore headers from ${BCoreHeaders}")

# add DLLs and Libs
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(BCoreDLL "${parentPath}/BCore/Build/Debug/BCore.dll")
    set(BCoreLib "${parentPath}/BCore/Build/Debug/BCore.lib")

else()
    set(BCoreDLL "${BUILD_PATH}/Debug/BCore.dll")
    set(BCoreLib "${BUILD_PATH}/Debug/BCore.lib")
endif()

set(All_Libs ${BCoreLib})
set(All_Headers ${BCoreHeaders})

add_executable(BCoreTest ${sources} ${headers})
add_custom_command(TARGET BCoreTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${BCoreDLL} 
    ${BUILD_PATH}/Debug
    COMMAND_EXPAND_LISTS
)

target_link_libraries(BCoreTest PUBLIC ${All_Libs})

target_include_directories(BCoreTest PUBLIC ${All_Headers})

# TODO 9: Use target_include_directories to include ${PROJECT_BINARY_DIR}