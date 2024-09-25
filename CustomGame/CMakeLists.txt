# TODO 1: Set the minimum required version of CMake to be 3.10
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(SET CMP0007 NEW)

include(${CMAKE_CURRENT_LIST_DIR}/../Utils.cmake)

project(CustomGame 
VERSION 1.0
DESCRIPTION "Game DLL" )

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

message("-- Headers included")
# add maths headers
cmake_path(GET PROJECT_BINARY_DIR PARENT_PATH parentPath)
cmake_path(GET parentPath PARENT_PATH parentPath)

# add BMath headers
set(mathHeaders "${parentPath}/BMaths")
message("\t-- Adding Maths headers from ${mathHeaders}")

# add BCore headers
set(bcoreHeaders "${parentPath}/BCore")
message("\t-- Adding BCore headers from ${bcoreHeaders}")

# add BCore headers
set(engineHeaders "${parentPath}/BEngine")
message("\t-- Adding BEngine headers from ${engineHeaders}")

# add Vulkan headers
set(vulkanHeaders "C:/Dev/Vulkan/Include")
message("\t-- Adding Vulkan headers from ${vulkanHeaders}")

# add Spir-V headers
set(spirVHeaders "C:/Dev/SPIRV-Reflect")
message("\t-- Adding SPIR-V headers from ${spirVHeaders}")
message("\r")

# add game headers
set(gameHeaders "${parentPath}/GameApp")
message("\t-- Adding game headers from ${gameHeaders}")
message("\r")

message("-- Static libraries included")

# add DLLs and Libs
if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(BCoreDLL "${parentPath}/BCore/Build/Debug/BCore.dll")
    set(BCoreLib "${parentPath}/BCore/Build/Debug/BCore.lib")

    set(BEngineDLL "${parentPath}/BEngine/Build/Debug/BEngine.dll")
    set(BEngineLib "${parentPath}/BEngine/Build/Debug/BEngine.lib")

else()
    set(BCoreDLL "${BUILD_PATH}/Debug/BCore.dll")
    set(BCoreLib "${BUILD_PATH}/Debug/BCore.lib")

    set(BEngineDLL "${BUILD_PATH}/Debug/BEngine.dll")
    set(BEngineLib "${BUILD_PATH}/Debug/BEngine.lib")   
endif()

message("\t-- Adding BCore DLL from ${BCoreDLL}")
message("\t-- Adding BEngine DLL from ${BEngineDLL}")

message("\r")

# add BEngine static lib

message("\r")

message("-- Build path")
message("\t-- ${BUILD_PATH}")
message("\r")
# TODO 3: Add an executable called Tutorial to the project
# Hint: Be sure to specify the source file as tutorial.cxx
add_library(CustomGameDLL SHARED ${sources} ${headers})
set_property(TARGET CustomGameDLL PROPERTY OUTPUT_NAME CustomGame)

target_link_libraries(CustomGameDLL "${BCoreLib}")
target_link_libraries(CustomGameDLL "${BEngineLib}")

target_include_directories(CustomGameDLL PUBLIC ${mathHeaders})
target_include_directories(CustomGameDLL PUBLIC ${bcoreHeaders})
target_include_directories(CustomGameDLL PUBLIC ${engineHeaders})
target_include_directories(CustomGameDLL PUBLIC ${gameHeaders})
target_include_directories(CustomGameDLL PUBLIC ${vulkanHeaders})
target_include_directories(CustomGameDLL PUBLIC ${spirVHeaders})

# TODO 9: Use target_include_directories to include ${PROJECT_BINARY_DIR}
# target_include_directories(ExecutableTarget PUBLIC "${PROJECT_BINARY_DIR}")
# add_library(BCore STATIC ${sources})
