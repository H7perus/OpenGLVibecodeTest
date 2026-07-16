#include "Model.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>





Model::Model(const std::string& path) {
    tinygltf::Model gltf_model;
    tinygltf::Scene scene;
    std::string err, warn;

    if (!tinygltf::TinyGLTF().LoadASCIIFromFile(&gltf_model, &err, &warn, path)) {
        std::cerr << "Failed to load model at: " << path << std::endl;
        if (!err.empty()) std::cerr << "Err: " << err << std::endl;
        if (!warn.empty()) std::cerr << "Warn: " << warn << std::endl;
        return;
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

            // Determine base stride from the first attribute we have (usually POSITION)
            int baseStride = 0;
            if (primitive.attributes.count("POSITION")) {
                const auto& posBV = gltf_model.bufferViews[gltf_model.accessors[primitive.attributes.at("POSITION")].bufferView];
                baseStride = (posBV.byteStride > 0) ? posBV.byteStride : (tinygltf::GetComponentSizeInBytes(gltf_model.accessors[primitive.attributes.at("POSITION")].componentType) * 3);
            } else if (primitive.attributes.count("NORMAL")) {
                const auto& normBV = gltf_model.bufferViews[gltf_model.accessors[primitive.attributes.at("NORMAL")].bufferView];
                baseStride = (normBV.byteStride > 0) ? normBV.byteStride : (tinygltf::GetComponentSizeInBytes(gltf_model.accessors[primitive.attributes.at("NORMAL")].componentType) * 3);
            } else if (primitive.attributes.count("TEXCOORD_0")) {
                const auto& texBV = gltf_model.bufferViews[gltf_model.accessors[primitive.attributes.at("TEXCOORD_0")].bufferView];
                baseStride = (texBV.byteStride > 0) ? texBV.byteStride : (tinygltf::GetComponentSizeInBytes(gltf_model.accessors[primitive.attributes.at("TEXCOORD_0")].componentType) * 2);
            }

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



            std::cout << "--- Vertices ---" << std::endl;
            for (size_t i = 0; i < vertices.size() / 8; ++i) {
                std::cout << "Vertex " << i << ": "
                          << vertices[i * 8 + 0] << ", " << vertices[i * 8 + 1] << ", " << vertices[i * 8 + 2] << " | "
                          << vertices[i * 8 + 3] << ", " << vertices[i * 8 + 4] << " | "
                          << vertices[i * 8 + 5] << ", " << vertices[i * 8 + 6] << ", " << vertices[i * 8 + 7] << std::endl;
            }

            if (primitive.indices != -1) {
                const tinygltf::Accessor& indexAccessor = gltf_model.accessors[primitive.indices];
                const auto& indexBV = gltf_model.bufferViews[indexAccessor.bufferView];
                const auto& indexBuffer = gltf_model.buffers[indexBV.buffer];

                std::cout << "--- Index Buffer Reconstruction ---" << std::endl;
                std::cout << "Buffer ID: " << indexBV.buffer << std::endl;
                std::cout << "BufferView Offset: " << indexBV.byteOffset << ", Stride: " << indexBV.byteStride << std::endl;
                std::cout << "Accessor Offset: " << indexAccessor.byteOffset << ", ComponentType: " << indexAccessor.componentType << std::endl;

                // Calculate the base pointer to where indices start in this buffer view
                const unsigned char* basePtr = indexBuffer.data.data() + indexBV.byteOffset + indexAccessor.byteOffset;
                std::cout << "Base Pointer: " << (void*)basePtr << std::endl;

                // Determine component size using tinygltf's helper function
                int componentSize = tinygltf::GetComponentSizeInBytes(indexAccessor.componentType);
                if (componentSize == -1) {
                    std::cerr << "WARNING: Unknown ComponentType detected: " << indexAccessor.componentType 
                              << ". Defaulting to 4 bytes for debugging." << std::endl;
                    componentSize = 4;
                }
                std::cout << "Detected Component Size: " << componentSize << " bytes" << std::endl;

                std::vector<uint32_t> indices;
                indices.reserve(indexAccessor.count);
                for (unsigned int i = 0; i < indexAccessor.count; ++i) {
                    size_t offset = (indexBV.byteStride > 0) ? (i * indexBV.byteStride) : (i * componentSize);
                    uint32_t val = 0;
                    if (componentSize == 2) {
                        val = *reinterpret_cast<const uint16_t*>(basePtr + offset);
                    } else if (componentSize == 4) {
                        val = *reinterpret_cast<const uint32_t*>(basePtr + offset);
                    } else {
                        val = 0; 
                    }
                    indices.push_back(val);
                }

                std::cout << "--- Indices ---" << std::endl;
                for (unsigned int i = 0; i < indices.size(); ++i) {
                    std::cout << "Index [" << i << "]: " << indices[i] << std::endl;
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
            std::cout << "Loaded mesh with " << m.indexCount << " indices." << std::endl;
        }
    }
}


void Model::Draw(unsigned int shaderID) {
    if (meshes.empty()) {
        std::cerr << "DEBUG ERROR: No meshes were loaded in the model!" << std::endl;
        return;
    }

    for (size_t i = 0; i < meshes.size(); ++i) {
        const auto& mesh = meshes[i];
        glBindVertexArray(mesh.VAO);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "DEBUG ERROR: Error before glDrawElements: " << err << std::endl;
        }

        if (mesh.EBO != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        }

        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "DEBUG ERROR: Error after glDrawElements: " << err << std::endl;
        }
    }
    glBindVertexArray(0);
}

Model::~Model() {
    for (const auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}