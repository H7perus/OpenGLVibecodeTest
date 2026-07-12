#include "Shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    unsigned int vertex = compileShader(GL_VERTEX_SHADER, vertexPath);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fragmentPath);

    if (vertex == 0 || fragment == 0) {
        std::cerr << "Shader compilation failed!" << std::endl;
        ID = 0;
        return;
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        std::cerr << "Error linking shader program!" << std::endl;
    }
}

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
    }
}

unsigned int Shader::compileShader(unsigned int type, const std::string& path) {
    std::string content;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening shader file: " << path << std::endl;
        return 0;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();

    const char* source = content.c_str();
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cerr << "Shader compilation failed: " << path << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}
