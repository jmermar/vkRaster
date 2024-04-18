# vkRaster

Simple Vulkan Renderer That I've began to work on. The purpose is to learn about the API and how to write efficient code on it.

# Features

The project has just started, right now it can only display a white triangle on screen and have some classes that will be useful later on.

It is supposed to contain the next features:
- Scene loading from gtlf files.
- GPU driven rendering.
- Frustum and occlusion culling.
- PBR materials.
- Defferred rendering.
- Postprocessing with tone-mapping.
- Bloom

# Build

The project is built with CMake and should run either on Windows or Linux (Ubuntu tested).
It has most of it's sources bundled with it, only depending on:
- GLM
- Vulkan (LunarG's Vulkan SDK required)

Instructions for Linux:

```
mkdir build &
cd build &
cmake .. &
make
```