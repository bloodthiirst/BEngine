# TODO 1: Set the minimum required version of CMake to be 3.10
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(SET CMP0007 NEW)

include(${CMAKE_CURRENT_LIST_DIR}/../Utils.cmake)

project(BEngine 
VERSION 1.0
DESCRIPTION "Engine layer" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# path to build files
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/Build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH})

SET(BUILD_MODE Debug)
add_compile_options("/Zc:preprocessor")
add_compile_definitions("_ENGINE_EXPORT")
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

# add game headers
set(gameHeaders "${parentPath}/GameApp")
message("\t-- Adding game headers from ${gameHeaders}")

# add Vulkan headers
set(vulkanHeaders "C:/Dev/Vulkan/Include")
message("\t-- Adding Vulkan headers from ${vulkanHeaders}")
message("\r")

message("-- Static libraries included")
# add BCore static lib
set(BCoreDLL "${parentPath}/BCore/Build/Debug/BCore.dll")
set(BCoreLib "${parentPath}/BCore/Build/Debug/BCore.lib")
message("\t-- Adding BCore DLL from ${BCoreDLL}")

# add vulkan static lib
set(VulkanLib "C:/Dev/Vulkan/Lib/vulkan-1.lib")
message("\t-- Adding Vulkan Static library from ${VulkanLib}")
message("\r")

message("-- Build path")
message("\t-- ${BUILD_PATH}")
message("\r")
# TODO 3: Add an executable called Tutorial to the project
# Hint: Be sure to specify the source file as tutorial.cxx

add_library(BEngineDLL SHARED ${sources} ${headers})
add_executable(BEngineExe ${sources} ${headers})

add_custom_command(TARGET BEngineExe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${BCoreDLL} 
    ${BUILD_PATH}/Debug
    COMMAND_EXPAND_LISTS
)

target_link_libraries(BEngineExe "${VulkanLib}")
target_link_libraries(BEngineExe "${BCoreLib}")

target_link_libraries(BEngineDLL "${VulkanLib}")
target_link_libraries(BEngineDLL "${BCoreLib}")

# Make both the .dll and .exe have the same name "BEngine"
set_property(TARGET BEngineExe PROPERTY OUTPUT_NAME BEngine)
set_property(TARGET BEngineDLL PROPERTY OUTPUT_NAME BEngine)

target_include_directories(BEngineExe PUBLIC ${mathHeaders})
target_include_directories(BEngineExe PUBLIC ${bcoreHeaders})
target_include_directories(BEngineExe PUBLIC ${gameHeaders})
target_include_directories(BEngineExe PUBLIC ${vulkanHeaders})

target_include_directories(BEngineDLL PUBLIC ${mathHeaders})
target_include_directories(BEngineDLL PUBLIC ${bcoreHeaders})
target_include_directories(BEngineDLL PUBLIC ${gameHeaders})
target_include_directories(BEngineDLL PUBLIC ${vulkanHeaders})

# TODO 9: Use target_include_directories to include ${PROJECT_BINARY_DIR}
# target_include_directories(ExecutableTarget PUBLIC "${PROJECT_BINARY_DIR}")
# add_library(BCore STATIC ${sources})
