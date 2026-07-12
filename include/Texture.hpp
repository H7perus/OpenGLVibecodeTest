#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>
#include <vector>
#include <glad/glad.h>

class Texture {
public:
    unsigned int ID;
    Texture(const std::string& path);
    ~Texture();

private:
    unsigned int loadTexture(const std::string& path);
};

#endif
