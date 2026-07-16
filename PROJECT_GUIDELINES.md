# Project Guidelines: OpenGLTriangle

## Tech Stack
- **Language:** C++17
- **Graphics API:** OpenGL
- **Windowing/Input:** SDL3
- **Loader:** GLAD
- **UI:** ImGui
- **Math:** GLM
- **Model Loading:** tiny_gltf
- **Image Loading:** stb_image

## Coding Standards
- **Style:** Modern C++. Prefer `std::unique_ptr` and `std::shared_ptr` for memory management where applicable.
- **RAII:** Ensure all GPU resources (Buffers, Textures, Shaders) are managed via RAII. Objects should clean up their own resources in the destructor.
- **Naming:** 
    - Classes: `PascalCase`
    - Methods/Functions: `camelCase`
    - Variables: `snake_case` or `camelCase` (be consistent, I'll stick to `camelCase` for methods).
- **Headers:** Keep headers lean. Include only what is strictly necessary. Use forward declarations where possible.
- **No Global State:** Avoid global variables. Pass dependencies (like the `Shader` object or `Texture` object) to the functions/classes that need them.

## Project Structure
- `include/`: Header files (.hpp)
- `src/`: Source files (.cpp)
- `shaders/`: GLSL vertex and fragment shader files
- `assets/`: Textures, models, and other external assets

## OpenGL Specifics
- Use `gladLoadGLLoader` for initializing GLAD.
- Ensure shaders are compiled and linked correctly before use.
- Use `glm` for all matrix and vector mathematics.

## Development Workflow
- When creating a new class, always create a corresponding `.hpp` in `include/` and `.cpp` in `src/`.
- When modifying the rendering loop, ensure state changes (like switching shaders or binding textures) are handled efficiently.
- If a new dependency is needed, check `vcpkg.json` first.