#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

class Shader;

enum class TextureType
{
    _diffuse,
    _specular
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoords;
};

struct Texture
{
    uint32_t    id;
    TextureType type;
};

class Mesh {

public:
    // mesh data
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture>  textures;

    Mesh(const std::vector<Vertex>&   vertices,
         const std::vector<uint32_t>& indices,
         const std::vector<Texture>&  textures);

    void Draw(Shader& shader);

private:
    // render data
    uint32_t VAO, VBO, EBO;

    void setupMesh();
};
