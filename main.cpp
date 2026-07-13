#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "include/Shader.hpp"
#include "include/Texture.hpp"

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenGL Cube", 800, 600, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "Context Creation Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Texture texture("assets/textures/container2.png");
    Texture specularTexture("assets/textures/container2_specular.png");

    std::vector<float> vertices;
    // Each face: 4 vertices (pos_x, pos_y, pos_z, tex_x, tex_y, norm_x, norm_y, norm_z)
    // Total 8 floats per vertex
    auto addFace = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 norm) {
        vertices.push_back(p1.x); vertices.push_back(p1.y); vertices.push_back(p1.z);
        vertices.push_back(0.0f);  vertices.push_back(0.0f);
        vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);

        vertices.push_back(p2.x); vertices.push_back(p2.y); vertices.push_back(p2.z);
        vertices.push_back(1.0f);  vertices.push_back(0.0f);
        vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);

        vertices.push_back(p3.x); vertices.push_back(p3.y); vertices.push_back(p3.z);
        vertices.push_back(1.0f);  vertices.push_back(1.0f);
        vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);

        vertices.push_back(p4.x); vertices.push_back(p4.y); vertices.push_back(p4.z);
        vertices.push_back(0.0f);  vertices.push_back(1.0f);
        vertices.push_back(norm.x); vertices.push_back(norm.y); vertices.push_back(norm.z);
    };

    // Front
    addFace(glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    // Back
    addFace(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f));
    // Top
    addFace(glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    // Bottom
    addFace(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f));
    // Right
    addFace(glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    // Left
    addFace(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f));

    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,  4, 5, 6, 4, 6, 7,  8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
    };

    unsigned int VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    glUseProgram(shader.ID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.ID);
    glUniform1i(glGetUniformLocation(shader.ID, "ourTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTexture.ID);
    glUniform1i(glGetUniformLocation(shader.ID, "ourSpecular"), 1);

    glUniform3f(glGetUniformLocation(shader.ID, "lightPos"), 0.0f, 5.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), 0.0f, 0.0f, 3.0f);

    bool running = true;
    SDL_Event event;
    Uint64 lastTick = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float deltaTime = (SDL_GetTicks() - lastTick) / 1000.0f;
        lastTick = SDL_GetTicks();

        model = glm::rotate(model, 0.2f * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, 0.2f * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
