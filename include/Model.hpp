#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <vector>
#include <glad/glad.h>
#include "Texture.hpp"

class Model {
public:
    Model(const std::string& path);
    ~Model();

    void Draw(unsigned int shaderID);

private:
    struct Mesh {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
        size_t indexCount;
    };

    std::vector<Mesh> meshes;
    // Add other members as needed (e.g., textures)
};

#endif