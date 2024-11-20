
# Raytracing

## Description
This project allows users to create basic 3D scenes using geometric objects like spheres and triangles. It leverages GPU processing for performance and includes features like image-based materials and camera controls. <br>
See the features section for more details.


## Dependencies
- [raylib](https://github.com/raysan5/raylib): easy to use graphics library
- [argparse](https://github.com/p-ranav/argparse): single header command line argument parser


## Getting Started
- Download / Clone the repository.
- Build raylib from source with `GRAPHICS_API=GRAPHICS_API_OPENGL_43` to use compute shaders.
- Download the `argparse` header as a dependency.
- In the root directory, run `make` to build the executable.
- Check the cli usage by passing `--help` as an argument to the executable.


### Project Structure
- `src/`: source code
    - `camera`: responsible for camera movement and projection matrix
    - `cli`: command line argument parser
    - `compiledscene`: packs scene data into proper format for raytracing
    - `hittable`: converts objects into gpu format
    - `logger`: simple logger utility
    - `material`: holds material data
    - `packedmaterialdata`: combines the materials into a single texture
    - `raytacer`: responsible for communicating with the gpu
    - `renderer`: responsible for rendering the final output
- `shaders/`: shaders
    - `packedmaterialgen`: generates the packed material texture for a scene
    - `raytracer`: compute shader that does the raytracing
    - `texFrag`: fragment shader for the rendering the final output


## Features
- **GPU Processing**: Runs on the GPU using compute shaders.
- **Camera Control**: Navigate through the 3D scene with camera controls.
- **Scene Creation**: Add objects like spheres and triangles to your scenes.
- **Image Materials**: Use images as materials for objects.
- **Texture Atlas**: Implement a texture atlas to improve efficiency.
- **Image Preprocessing**: Includes gamma correction for better color representation. (WIP)


### TODO
- **Depth of Field**: Add support for depth of field.
- **Easy Scene Creation**: Create scenes using json files.
- **Proper Image Processing**: Improve image preprocessing steps to ensure accurate color representation.
- **Advanced Materials**: Currently materials includes an albedo coloor and a roughness value. Add support for other material properties like normals, specular, etc.
- **Mesh Support**: Add support for meshes. Make formats for passing mesh data to the shaders.
- **Acceleration Structures**: Implement acceleration structures like BVH or Octrees to improve performance.
