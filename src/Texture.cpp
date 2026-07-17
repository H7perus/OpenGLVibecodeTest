#include "Texture.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>

#include <stb_image.h>

Texture::Texture(const std::string& path) {
    ID = loadTexture(path);
}

// New constructor for embedded RGBA pixel data (already decoded by tiny_gltf)
Texture::Texture(unsigned char* imageData, int width, int height) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    if (imageData == nullptr || width <= 0 || height <= 0) {
        std::cerr << "[Texture] ERROR: Null or invalid image dimensions" << std::endl;
        glDeleteTextures(1, &textureID);
        return;
    }
    
    // tiny_gltf stores decoded RGBA pixel data (not PNG bytes)
    // Upload directly with GL_RGBA format - OpenGL handles the channel order correctly
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    // Set texture parameters for nearest-neighbor filtering (no mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    ID = textureID;
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    if (ID != 0) {
        glDeleteTextures(1, &ID);
    }
}

unsigned int Texture::loadTexture(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int alpha;
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data == nullptr) {
        std::cerr << "Failed to load texture at: " << path << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return textureID;
}