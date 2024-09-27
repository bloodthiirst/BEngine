# BEngine
Experimenting with C++ and game engine architecture
The aim with this project is to build an engine from scratch using the least (if not 0) dependencies possible.

The code is pretty much C style with minimal use of templates (for container types) and operator overload for Maths.
The engine uses Win32 API and Vulkan as a rendering backend.

# Overview
The project is split into different components
- BCore : Provides common datastructures and algorithms (containers , allocators , cutstom string , serializers)
- BEngine : The engine containing the specific engine components (input , window , rendering backend , logging)
- BMaths : Custom math library
- GameApp : a simple header file containing the client library that will represent the game and use the engine as a dependency
- CustomGame : a simple example of a client DLL using the engine

# Dependencies
- CMake : for building the project
- Vulkan : for the rendering backend

# Build
- Windows (only for now) : simply run `clean-build.bat` for a full CMake rebuild , then you can run `build.bat` to build , for more custom behaviour feel free to inspect/change the `CMakeLists.txt` file.

# Implemented Features
- A propriatary implementations of basic containers (with more to come)
  - DArray (dynamic arrays)
  - HMap (Hashmap - Dictionary)
  - ArrayView
  - LinkedList
  - Stack
  - FreeList
- StringView , StringBuffer and StringBuilder instead of C-style null terminated strings
- Support for custom allocators for all allocation related operations
- Custom Math library (with plans to support SIMD)
- Lightweight JSON serializer
- Lightweight XML serializer

# Basic Client DLL with FPS camera and shaded mesh
![alt text](https://github.com/bloodthiirst/BEngine/blob/master/Screenshot.png?raw=true)
