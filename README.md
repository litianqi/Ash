Ash is a tiny game engine based on Vulkan. I use it as a playground for learning modern rendering techniques, such as bindless, GPU driven rendering, multithreaded rendering, frame graph (render graph), mesh shader, ray tracing, etc.

## Directory Structure

```txt
📦Ash
 ┣ 📂resources         -- texture, model, etc.
 ┣ 📂samples           -- sample source code
 ┣ 📂shaders           -- shaders
 ┣ 📂src
 ┃ ┣ 📂editor          -- editor source code 
 ┃ ┗ 📂runtime         -- ash library source code
 ┣ 📂tests             -- unit test soruce code
 ┗ 📜CMakeLists.txt
```

## Dependencies

Ash uses the following open-source libraries:

- [lightweightvk](https://github.com/litianqi/lightweightvk)
- [SDL3](https://github.com/libsdl-org/SDL)
- [glm](https://github.com/icaven/glm)
- [spdlog](https://github.com/gabime/spdlog)
- [SG14](https://github.com/WG21-SG14/SG14/tree/master/SG14)
- [stb](https://github.com/nothings/stb)
- [simdjson](https://github.com/simdjson/simdjson)
- [fastgltf](https://github.com/spnda/fastgltf)
- [OffsetAllocator](https://github.com/sebbbi/OffsetAllocator)

## Samples

| Sample Name    | Screenshot                                             | Description                             |
|----------------|--------------------------------------------------------|-----------------------------------------|
| Hello Window   | <img src="screenshots/hello_window.png" width="300">   | Create and show a SDL window.           |
| Hello Triangle | <img src="screenshots/hello_triangle.png" width="300"> | Render a triangle                       |
| Hello Cube     | <img src="screenshots/hello_cube.png" width="300">     | Render a number of self rotating cubes. |
| Hello Camera   | <img src="screenshots/hello_camera.png" width="300">   | Add fly and orbit camera controller.    |
| Hello glTF     | <img src="screenshots/hello_gltf.png" width="300">     | Load and render a glTF file.            |
