#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glad/glad.h>

class Shader {
public:
    unsigned int ID;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

private:
    unsigned int compileShader(unsigned int type, const std::string& path);
};

#endif