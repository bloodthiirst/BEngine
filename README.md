# BEngine
Experimenting with C++ and game engine architecture
The aim with this project is to build an engine from scratch using the least (if not 0) dependencies possible.

The code is pretty much C style with minimal use of templates (for container types) and operator overload for Maths.
The engine uses Win32 API and Vulkan as a rendering backend.

# Implemented Features
- A propriatary implementations of basic containers (with more to come)
  - DArray (dynamic arrays)
  - HMap (Hashmap - Dictionary)
- StringView an StringBuffer instead of C-style null terminated strings
- Support for custom allocators for all allocation related operations

# First Triangle
![alt text](https://github.com/bloodthiirst/BEngine/blob/master/first_triangle.PNG?raw=true)
