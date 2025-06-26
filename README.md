# DiaBolic

A barebones project I made to learn basic DX12 and CMake with. It has set out to become a bit of a framework along the way and I'd like to extend it to make a dummy renderer for any future experimental graphics features I'd like to explore.
Screenshot of the current state:
![image](https://github.com/user-attachments/assets/2db4957a-383d-4ab9-b6d1-f0192cf01e3e)


#### It features:
- A complete bindless model (for buffers + textures)
- DXC shader compilation
- Basic wrappers for command queues and descriptor heaps

#### Future plans:
- Async/multi-threaded model loading using Assimp
- Deferred rendering pipeline with PBR lighting

## Getting started
### Prerequisites
Make sure to have these installed:
- Visual Studio Code
- CMake
- MSVC compiler
- Ninja
- Git
VS Code Extensions:
- CMake Tools
- C++ Externsion

### One-Time Setup 
Run the following commands in the project root's terminal:
- `git submodule update --init --recursive`
- `.\vcpkg\bootstrap-vcpkg.bat`
- `.\vcpkg\vcpkg install`

### Running the Project in VS Code
- `Ctrl+Shift+P` -> CMake: Select Configure Preset, then choose: x64 Debug or x64 Release
- `Ctrl+Shift+P` -> CMake: Configure
- `Ctrl+Shift+P` -> CMake: Build
- `Ctrl+Shift+D` -> Launch Debug or Launch Release

### Updating Dependencies
If you add new dependencies to `vcpkg.json`, rerun `.\vcpkg\vcpkg install`

## Learning resources
Some of the most notable learning resources and references used:
- https://www.3dgep.com/category/graphics-programming/directx/
- https://rtarun9.github.io/blogs/bindless_rendering/ and their https://github.com/rtarun9/Helios/tree/master
