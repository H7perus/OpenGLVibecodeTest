#include "Model.hpp"
#include <iostream>
#include <cstring>  // For memcpy
#include <vector>   // For std::vector

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>


Model::Model(const std::string& path) {
    tinygltf::Model gltf_model;
    tinygltf::Scene scene;
    std::string err, warn;

    // Determine file type based on extension and load accordingly
    bool isBinary = (path.size() > 4) && (path.substr(path.size() - 4) == ".glb");
    
    if (isBinary) {
        // Load binary glTF (.glb) format - same signature as LoadASCIIFromFile
        if (!tinygltf::TinyGLTF().LoadBinaryFromFile(&gltf_model, &err, &warn, path)) {
            std::cerr << "Failed to load model at: " << path << std::endl;
            if (!err.empty()) std::cerr << "Err: " << err << std::endl;
            if (!warn.empty()) std::cerr << "Warn: " << warn << std::endl;
            return;
        }
    } else {
        // Load ASCII glTF (.gltf) format
        if (!tinygltf::TinyGLTF().LoadASCIIFromFile(&gltf_model, &err, &warn, path)) {
            std::cerr << "Failed to load model at: " << path << std::endl;
            if (!err.empty()) std::cerr << "Err: " << err << std::endl;
            if (!warn.empty()) std::cerr << "Warn: " << warn << std::endl;
            return;
        }
    }

    // Iterate over all meshes in the glTF model
    for (const tinygltf::Mesh& mesh : gltf_model.meshes) {
        for (const tinygltf::Primitive& primitive : mesh.primitives) {
            Mesh m;

            size_t numVertices = 0;
            if (primitive.attributes.count("POSITION")) {
                const int posAttrIdx = primitive.attributes.at("POSITION");
                const tinygltf::Accessor& posAccessor = gltf_model.accessors[posAttrIdx];
                const auto& posBV = gltf_model.bufferViews[posAccessor.bufferView];
                int stride = (posBV.byteStride > 0) ? posBV.byteStride : (tinygltf::GetComponentSizeInBytes(posAccessor.componentType) * 3);
                numVertices = std::min((size_t)posAccessor.count, (size_t)(posBV.byteLength / stride));
            }

            if (numVertices == 0) continue;

            std::vector<float> vertices(numVertices * 8);

            // Pack Position
            if (primitive.attributes.count("POSITION")) {
                const int posAttrIdx = primitive.attributes.at("POSITION");
                const tinygltf::Accessor& posAccessor = gltf_model.accessors[posAttrIdx];
                const auto& posBV = gltf_model.bufferViews[posAccessor.bufferView];
                const auto& posBuffer = gltf_model.buffers[posBV.buffer];
                int stride = (posBV.byteStride > 0) ? posBV.byteStride : (tinygltf::GetComponentSizeInBytes(posAccessor.componentType) * 3);

                for (size_t i = 0; i < numVertices; ++i) {
                    size_t offset = posBV.byteOffset + posAccessor.byteOffset + i * stride;
                    const float* data = reinterpret_cast<const float*>(&posBuffer.data[offset]);
                    vertices[i * 8 + 0] = data[0];
                    vertices[i * 8 + 1] = data[1];
                    vertices[i * 8 + 2] = data[2];
                }
            }

            // Pack TexCoord
            if (primitive.attributes.count("TEXCOORD_0")) {
                const int texAttrIdx = primitive.attributes.at("TEXCOORD_0");
                const tinygltf::Accessor& texAccessor = gltf_model.accessors[texAttrIdx];
                const auto& texBV = gltf_model.bufferViews[texAccessor.bufferView];
                const auto& texBuffer = gltf_model.buffers[texBV.buffer];
                int stride = (texBV.byteStride > 0) ? texBV.byteStride : (tinygltf::GetComponentSizeInBytes(texAccessor.componentType) * 2);

                for (size_t i = 0; i < numVertices; ++i) {
                    size_t offset = texBV.byteOffset + texAccessor.byteOffset + i * stride;
                    const float* data = reinterpret_cast<const float*>(&texBuffer.data[offset]);
                    vertices[i * 8 + 3] = data[0];
                    vertices[i * 8 + 4] = data[1];
                }
            }

            // Pack Normal
            if (primitive.attributes.count("NORMAL")) {
                const int normAttrIdx = primitive.attributes.at("NORMAL");
                const tinygltf::Accessor& normAccessor = gltf_model.accessors[normAttrIdx];
                const auto& normBV = gltf_model.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = gltf_model.buffers[normBV.buffer];
                int stride = (normBV.byteStride > 0) ? normBV.byteStride : (tinygltf::GetComponentSizeInBytes(normAccessor.componentType) * 3);

                for (size_t i = 0; i < numVertices; ++i) {
                    size_t offset = normBV.byteOffset + normAccessor.byteOffset + i * stride;
                    const float* data = reinterpret_cast<const float*>(&normBuffer.data[offset]);
                    vertices[i * 8 + 5] = data[0];
                    vertices[i * 8 + 6] = data[1];
                    vertices[i * 8 + 7] = data[2];
                }
            }

            // Load albedo texture from material (if available)
            Texture* albedoTex = nullptr;
            
            if (primitive.material != -1 && static_cast<size_t>(primitive.material) < gltf_model.materials.size()) {
                const tinygltf::Material& mat = gltf_model.materials[primitive.material];
                
                // Check for baseColorTexture first (embedded or external texture)
                // In glTF, the texture is stored in material.pbrMetallicRoughness.baseColorTexture.index
                if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    int imageIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
                    
                    if (static_cast<size_t>(imageIdx) < gltf_model.images.size()) {
                        const tinygltf::Image& img = gltf_model.images[imageIdx];
                        
                        // Check if texture data is embedded in the glTF file
                        std::vector<unsigned char> texData;
                        
                        if (!img.image.empty()) {
                            // tiny_gltf stores decoded RGBA pixel data (not PNG bytes)
                            int width = img.width > 0 ? static_cast<int>(img.width) : 1;
                            int height = img.height > 0 ? static_cast<int>(img.height) : 1;
                            
                            texData.assign(img.image.begin(), img.image.end());
                            
                            if (width > 0 && height > 0) {
                                albedoTex = new Texture(texData.data(), width, height);
                                if (albedoTex->ID != 0) {
                                    std::cout << "[DEBUG] Loaded embedded texture ID=" << albedoTex->ID << std::endl;
                                } else {
                                    std::cerr << "[ERROR] FAILED to load embedded texture" << std::endl;
                                }
                            } else {
                                // Could not determine dimensions - fall back to baseColorFactor
                                const auto& factor = mat.pbrMetallicRoughness.baseColorFactor;
                                texData.resize(4);
                                texData[0] = static_cast<unsigned char>(factor[0] * 255.0f);
                                texData[1] = static_cast<unsigned char>(factor[1] * 255.0f);
                                texData[2] = static_cast<unsigned char>(factor[2] * 255.0f);
                                texData[3] = static_cast<unsigned char>(factor[3] * 255.0f);
                                albedoTex = new Texture(texData.data(), 1, 1);
                            }
                        } else {
                            // No embedded data - tiny_gltf should have already handled external textures during load
                            // Create a 64x64 magenta/black grid fallback texture
                            std::cout << "[DEBUG] Using magenta/black grid fallback texture" << std::endl;
                            albedoTex = createGridTexture();
                        }
                    }
                }
                
                // If no texture loaded, check baseColorFactor and create 1x1 texture
                if (!albedoTex) {
                    const auto& factor = mat.pbrMetallicRoughness.baseColorFactor;
                    
                    // Create a temporary buffer for the 1x1 texture data (RGBA)
                    std::vector<unsigned char> texData(4);
                    texData[0] = static_cast<unsigned char>(factor[0] * 255.0f);  // R
                    texData[1] = static_cast<unsigned char>(factor[1] * 255.0f);  // G
                    texData[2] = static_cast<unsigned char>(factor[2] * 255.0f);  // B
                    texData[3] = static_cast<unsigned char>(factor[3] * 255.0f);  // A
                    
                    albedoTex = new Texture(texData.data(), 1, 1);
                }
            } else {
                // No material assigned to primitive - use white fallback
                std::vector<unsigned char> texData(4);
                texData[0] = 255; texData[1] = 255; texData[2] = 255; texData[3] = 255;
                
                albedoTex = new Texture(texData.data(), 1, 1);
            }

            // Store texture pointer in mesh struct
            m.albedoTexturePtr = albedoTex;

            if (primitive.indices != -1) {
                const tinygltf::Accessor& indexAccessor = gltf_model.accessors[primitive.indices];
                const auto& indexBV = gltf_model.bufferViews[indexAccessor.bufferView];
                const auto& indexBuffer = gltf_model.buffers[indexBV.buffer];

                // Determine component size using tinygltf's helper function
                int componentSize = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType);
                if (componentSize == -1) {
                    std::cerr << "WARNING: Unknown ComponentType detected: " << indexAccessor.componentType 
                              << ". Defaulting to 4 bytes for debugging." << std::endl;
                    componentSize = 4;
                }

                std::vector<uint32_t> indices;
                indices.reserve(indexAccessor.count);
                for (unsigned int i = 0; i < indexAccessor.count; ++i) {
                    size_t offset = (indexBV.byteStride > 0) ? (i * indexBV.byteStride) : (i * componentSize);
                    uint32_t val = 0;
                    if (componentSize == 2) {
                        val = *reinterpret_cast<const uint16_t*>(indexBuffer.data.data() + indexBV.byteOffset + indexAccessor.byteOffset + offset);
                    } else if (componentSize == 4) {
                        val = *reinterpret_cast<const uint32_t*>(indexBuffer.data.data() + indexBV.byteOffset + indexAccessor.byteOffset + offset);
                    } else {
                        val = 0; 
                    }
                    indices.push_back(val);
                }

                glGenBuffers(1, &m.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), 
                              indices.data(), GL_STATIC_DRAW);
                m.indexCount = indices.size();
            }
            // Normal: 3 floats, starting at offset 5*sizeof(float)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

            glGenVertexArrays(1, &m.VAO);
            glBindVertexArray(m.VAO);

            if (primitive.indices != -1) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
            }

            glGenBuffers(1, &m.VBO);
            glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);


            // Position: location 0 (3 floats)
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));

            // TexCoord: location 1 (2 floats)
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

            // Normal: location 2 (3 floats)
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);


            meshes.push_back(m);
            
            // Add texture to vector if it exists
            if (albedoTex) {
                albedoTextures.push_back(*albedoTex);
            }
        }
    }
}

void Model::Draw(unsigned int shaderID) {
    for (const auto& mesh : meshes) {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderID, "u_albedoTexture"), 0);
        glBindTexture(GL_TEXTURE_2D, mesh.albedoTexturePtr->ID);

        glBindVertexArray(mesh.VAO);

        if (mesh.EBO != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        }

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

Model::~Model() {
    // Delete texture pointers in each mesh
    for (const auto& mesh : meshes) {
        if (mesh.albedoTexturePtr) {
            delete mesh.albedoTexturePtr;
        }
    }

    glDeleteVertexArrays(static_cast<GLsizei>(meshes.size()), reinterpret_cast<const GLuint*>(meshes.data()));
    glDeleteBuffers(static_cast<GLsizei>(meshes.size()), reinterpret_cast<const GLuint*>(meshes.data()));
    glDeleteBuffers(static_cast<GLsizei>(meshes.size()), reinterpret_cast<const GLuint*>(meshes.data()));
}

// Create a 64x64 magenta/black grid fallback texture
Texture* Model::createGridTexture() {
    const int gridSize = 8;
    const int texSize = 64;
    
    // Generate grid pattern data (magenta on black)
    std::vector<unsigned char> gridData(texSize * texSize * 4);
    
    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            int idx = (y * texSize + x) * 4;
            
            // Calculate grid cell position
            int cellX = x / gridSize;
            int cellY = y / gridSize;
            
            // Check if this is a magenta cell (even cells) or black (odd cells)
            bool isMagenta = (cellX + cellY) % 2 == 0;
            
            if (isMagenta) {
                // Magenta color: R=255, G=0, B=255, A=255
                gridData[idx + 0] = 255;   // R
                gridData[idx + 1] = 0;     // G
                gridData[idx + 2] = 255;   // B
                gridData[idx + 3] = 255;   // A
            } else {
                // Black color: R=0, G=0, B=0, A=255
                gridData[idx + 0] = 0;     // R
                gridData[idx + 1] = 0;     // G
                gridData[idx + 2] = 0;     // B
                gridData[idx + 3] = 255;   // A
            }
        }
    }
    
    return new Texture(gridData.data(), texSize, texSize);
}