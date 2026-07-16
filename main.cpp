#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "include/Shader.hpp"
#include "include/Texture.hpp"
#include "include/Model.hpp"

// Create ImGui context at startup
static ImGuiContext* g_pImGuiCtx = nullptr;

// Global UI state variables (moved from window scope to prevent crashes)
static float cameraYaw = 0.0f;      // Horizontal rotation angle
static float cameraPitch = 0.0f;    // Vertical rotation angle  
static float orbitDistance = 3.0f;  // Distance from center for orbiting
static float lightHeight = 5.0f;
static bool showGrid = false;
static bool useSpecular = true;

// Mouse input tracking for orbit controls
static int lastMouseX = 0;
static int lastMouseY = 0;
static Uint32 lastMouseTime = 0;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("OpenGL Cube with ImGui", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

    // Create and setup ImGui context
    g_pImGuiCtx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Add default font and build atlas before initializing backends
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    // Initialize ImGui with SDL3 OpenGL backend
    if (!ImGui_ImplSDL3_InitForOpenGL(window, context)) {
        std::cerr << "Failed to initialize ImGui SDL3 backend" << std::endl;
        return 1;
    }
    ImGui_ImplOpenGL3_Init("#version 150");

    glEnable(GL_DEPTH_TEST);
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Texture texture("assets/textures/container2.png");
    Texture specularTexture("assets/textures/container2_specular.png");

    Model model("assets/models/char.gltf");

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

    glm::mat4 modelMatrix = glm::mat4(1.0f);
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

    // Mouse state tracking for orbit controls
    static bool mouseDown = false;
    static int mouseX = 0;
    static int mouseY = 0;
    static float lastYaw = cameraYaw;
    static float lastPitch = cameraPitch;

    while (running) {
        // Handle events and process them through ImGui
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            
            if (event.type == SDL_EVENT_QUIT || 
                event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
            
            // Mouse button events for orbit controls
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                mouseDown = true;
                mouseX = event.button.x;
                mouseY = event.button.y;
                lastYaw = cameraYaw;
                lastPitch = cameraPitch;
            }
            
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
                mouseDown = false;
            }
            
            // Mouse motion for orbit controls
            if (event.type == SDL_EVENT_MOUSE_MOTION && mouseDown) {
                int currentX = event.motion.x;
                int currentY = event.motion.y;
                
                float deltaX = currentX - mouseX;
                float deltaY = currentY - mouseY;
                
                // Convert pixel movement to rotation angles (degrees per pixel)
                cameraYaw += deltaX * 0.5f;
                cameraPitch -= deltaY * 0.5f;
                
                // Clamp pitch to prevent flipping over
                if (cameraPitch > 89.0f) cameraPitch = 89.0f;
                if (cameraPitch < -89.0f) cameraPitch = -89.0f;
                
                mouseX = currentX;
                mouseY = currentY;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame (SDL3 + OpenGL3 backend handles rendering)
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Build font atlas if not already built
        ImGuiIO& io = ImGui::GetIO();
        if (!io.Fonts->IsBuilt()) {
            io.Fonts->AddFontDefault();
            io.Fonts->Build();
        }

        float deltaTime = (SDL_GetTicks() - lastTick) / 1000.0f;
        lastTick = SDL_GetTicks();

        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Orbit camera: translate first then rotate (camera position in world space)
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -orbitDistance));
        view = glm::rotate(view, glm::radians(cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f));

        model.Draw(shader.ID);

        // Manual cube draw calls commented out as we are using the loaded model
        // glBindVertexArray(VAO);
        // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Draw ImGui UI elements
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    running = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                static bool showWireframe = false;
                if (ImGui::MenuItem("Wireframe", "Ctrl+Z")) {
                    std::cout << "Toggle wireframe" << std::endl;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Camera Controls panel (combined rotation + distance)
        if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Orbit Controls section
            ImGui::PushID("OrbitControls");
            ImGui::Text("Orbit Camera:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Left Click + Drag to rotate");
            
            ImGui::SliderFloat("Yaw (Horizontal)", &cameraYaw, -360.0f, 360.0f);
            ImGui::SliderFloat("Pitch (Vertical)", &cameraPitch, -90.0f, 90.0f);
            ImGui::PopID();

            ImGui::Separator();

            ImGui::SliderFloat("Orbit Distance", &orbitDistance, 1.0f, 15.0f);

            ImGui::Separator();

            if (ImGui::Checkbox("Show Grid", &showGrid)) {
                std::cout << "Grid visibility toggled" << std::endl;
            }

            ImGui::End();
        } else {
            // Window is collapsed - still need to call End() but with no content
            ImGui::End();
        }

        // FPS counter and performance info (outside window scope to avoid crash on fold)
        static int frameCount = 0;
        static float fps = 0.0f;
        if (++frameCount == 60) {
            fps = 1.0f / deltaTime * frameCount / 60.0f;
            frameCount = 0;
        }

        // Update lighting position from UI controls (use global variables, not static in window)
        glUniform3f(glGetUniformLocation(shader.ID, "lightPos"), 0.0f, lightHeight, 0.0f);
        
        if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("FPS: %.1f", fps);
            ImGui::Separator();
            ImGui::Text("Model: char.gltf");
            ImGui::Text("Textures: container2.png, specular.png");
            
            ImGui::End();
        } else {
            // Window is collapsed - still need to call End() but with no content
            ImGui::End();
        }

        // Lighting controls panel
        if (ImGui::Begin("Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SliderFloat("Light Height", &lightHeight, 1.0f, 10.0f);

            ImGui::Separator();

            if (ImGui::Checkbox("Use Specular Mapping", &useSpecular)) {
                glUniform1i(glGetUniformLocation(shader.ID, "ourSpecular"),
                    useSpecular ? 1 : 0);
            }

            ImGui::End();
        } else {
            // Window is collapsed - still need to call End() but with no content
            ImGui::End();
        }

        // Render ImGui (SDL3 + OpenGL3 backend handles rendering)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Shutdown ImGui
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();

    return 0;
}
