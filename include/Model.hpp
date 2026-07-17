#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <vector>
#include <vector>
#include <glad/glad.h>
#include "Texture.hpp"

class Model {
public:
    Model(const std::string& path);
    ~Model();

    void Draw(unsigned int shaderID);
    
    Texture* createGridTexture();  // Helper for fallback texture generation

private:
    struct Mesh {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
        size_t indexCount;
        Texture* albedoTexturePtr;  // Pointer to per-mesh albedo texture (can be nullptr)
    };

    std::vector<Mesh> meshes;
    std::vector<Texture> albedoTextures;  // Per-mesh albedo textures
};

#endif