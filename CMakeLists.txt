cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)

project(OpenGLTriangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui REQUIRED)

# Find tiny_gltf include directories if not automatically handled
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
if(NOT TINYGLTF_INCLUDE_DIRS)
    message(FATAL_ERROR "tiny_gltf headers not found")
endif()

add_executable(${PROJECT_NAME} main.cpp src/Shader.cpp src/Texture.cpp src/Model.cpp src/stb_impl.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE include ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 OpenGL::GL glad::glad glm::glm imgui::imgui)
