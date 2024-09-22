# BEngine
Experimenting with C++ and game engine architecture
The aim with this project is to build an engine from scratch using the least (if not 0) dependencies possible.

The code is pretty much C style with minimal use of templates (for container types) and operator overload for Maths.
The engine uses Win32 API and Vulkan as a rendering backend.

# Implemented Features
- A propriatary implementations of basic containers (with more to come)
  - DArray (dynamic arrays)
  - HMap (Hashmap - Dictionary)
  - ArrayView
  - LinkedList
  - FreeList
- StringView , StringBuffer and StringBuilder instead of C-style null terminated strings
- Support for custom allocators for all allocation related operations
- Custom Math library (with plans to support SIMD)
- Lightweight JSON serializer

# Basic Client DLL with FPS camera and shaded mesh
![alt text](https://github.com/bloodthiirst/BEngine/blob/master/Screenshot.png?raw=true)
