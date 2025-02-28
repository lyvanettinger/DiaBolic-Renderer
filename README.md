# DiaBolic

A barebones project I made to learn basic DX12 (and eventually font rendering) with. It has set out to become a bit of a framework along the way and I'd like to extend it to make a dummy renderer for any future experimental graphics features I'd like to explore.
Screenshot of the current state:
![image](https://github.com/user-attachments/assets/2db4957a-383d-4ab9-b6d1-f0192cf01e3e)


#### It features:
- A complete bindless model (for buffers + textures)
- DXC shader compilation
- Basic wrappers for command queues and descriptor heaps

#### Future plans:
- Model loading (GLTF/fbx)
- Deferred rendering pipeline with PBR lighting

## How to build

- Build project with CMake
- Run Setup.bat

If the compiler is complaining about git submodules, run
`git submodule update --init --recursive`.

## Learning resources
Some of the most notable learning resources and references used:
- https://www.3dgep.com/category/graphics-programming/directx/
- https://rtarun9.github.io/blogs/bindless_rendering/ and their https://github.com/rtarun9/Helios/tree/master
