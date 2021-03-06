#include <glad/glad.h>

#include "mesh.h"
#include "shader.h"

Mesh::Mesh(const std::vector<Vertex>&   vertices,
           const std::vector<uint32_t>& indices,
           const std::vector<Texture>&  textures)
{
    this->vertices = vertices;
    this->indices  = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    // vertex positons
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoords));

    glBindVertexArray(0);
}

void Mesh::Draw(Shader& shader)
{
    uint32_t diffuseNr  = 1;
    uint32_t specularNr = 1;

    for (size_t index = 0; index < textures.size(); index++)
    {
        glActiveTexture(GL_TEXTURE0 + (int32_t)index);

        std::string number, name;
        if (textures[index].type == TextureType::_diffuse)
        {
            number = std::to_string(diffuseNr++);
            name   = "diffuse";
        }
        else if (textures[index].type == TextureType::_specular)
        {
            number = std::to_string(specularNr++);
            name   = "specular";
        }

        shader.setFloat(("material." + name + number).c_str(), static_cast<float>(index));
        glBindTexture(GL_TEXTURE_2D, textures[index].id);
    }

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (uint32_t)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
