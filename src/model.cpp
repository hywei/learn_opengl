#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <stb_image/stb_image.h>

#include <iostream>

#include "model.h"
#include "shader.h"

uint32_t TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

Model::Model(const char* path)
{
    loadModel(path);
}

Model::~Model()
{
    for (auto* mesh : meshes_)
    {
        delete mesh;
    }
}

void Model::Draw(Shader& shader)
{
    for (auto* mesh : meshes_)
    {
        mesh->Draw(shader);
    }
}

void Model::loadModel(std::string path)
{
    Assimp::Importer importer;
    const aiScene*   scene =
        importer.ReadFile(path,
                          aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                              aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory_ = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (uint32_t index = 0; index < node->mNumMeshes; index++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[index]];
        meshes_.push_back(processMesh(mesh, scene));
    }

    for (uint32_t index = 0; index < node->mNumChildren; index++)
    {
        processNode(node->mChildren[index], scene);
    }
}

Mesh* Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture>  textures;

    for (uint32_t index = 0; index < mesh->mNumVertices; index++)
    {
        Vertex new_vertex;

        // positions
        new_vertex.position.x = mesh->mVertices[index].x;
        new_vertex.position.y = mesh->mVertices[index].y;
        new_vertex.position.z = mesh->mVertices[index].z;

        // normals
        if (mesh->HasNormals())
        {
            new_vertex.normal.x = mesh->mNormals[index].x;
            new_vertex.normal.y = mesh->mNormals[index].y;
            new_vertex.normal.z = mesh->mNormals[index].z;
        }

        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            // a vertex can contain up to 8 different texture coordinates. We thus make the
            // assumption that we won't use models where a vertex can have multiple txture
            // coordinates so we always take the first set (0).
            new_vertex.texcoords.x = mesh->mTextureCoords[0][index].x;
            new_vertex.texcoords.y = mesh->mTextureCoords[0][index].y;

            // tangent
            new_vertex.tangent.x = mesh->mTangents[index].x;
            new_vertex.tangent.y = mesh->mTangents[index].y;

            // bitangent
            new_vertex.bitangent.x = mesh->mBitangents[index].x;
            new_vertex.bitangent.y = mesh->mBitangents[index].y;
        }
        else
        {
            new_vertex.texcoords = glm::vec2(0.f, 0.f);
        }

        vertices.push_back(new_vertex);
    }

    for (uint32_t index = 0; index < mesh->mNumFaces; index++)
    {
        for (uint32_t j = 0; j < mesh->mFaces[index].mNumIndices; j++)
        {
            indices.push_back(mesh->mFaces[index].mIndices[j]);
        }
    }

    // process materials
    if (mesh->mMaterialIndex >= 0)
    {
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be
        // named as 'texture_diffuseN' where N is a sequential number ranging from 1 to
        // MAX_SAMPLER_NUMBER. Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuesN
        // specular: texture_specularN
        // normal: texture_normalN

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // 1. diffuse maps
        std::vector<Texture> diffuse_maps =
            loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::_diffuse);
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        // 2. specular maps
        std::vector<Texture> specular_maps =
            loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::_specular);
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

        // 3. normal maps
        std::vector<Texture> normal_maps =
            loadMaterialTextures(material, aiTextureType_NORMALS, TextureType::_normal);
        textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());

        // 4. height maps
        std::vector<Texture> height_maps =
            loadMaterialTextures(material, aiTextureType_AMBIENT, TextureType::_height);
        textures.insert(textures.end(), height_maps.begin(), height_maps.end());
    }

    Mesh* new_mesh = new Mesh(vertices, indices, textures);

    return new_mesh;
}

std::vector<Texture>
Model::loadMaterialTextures(aiMaterial* mat, uint32_t ai_texture_type, TextureType texture_type)
{
    std::vector<Texture> textures;
    for (uint32_t index = 0;
         index < mat->GetTextureCount(static_cast<aiTextureType>(ai_texture_type));
         index++)
    {
        aiString str;
        mat->GetTexture(static_cast<aiTextureType>(ai_texture_type), index, &str);

        if (isTextureLoaded(str.C_Str()))
            continue;

        // new texture
        Texture texture;
        texture.id   = TextureFromFile(str.C_Str(), directory_);
        texture.type = texture_type;
        texture.path = str.C_Str();
        textures.push_back(texture);

        loaded_textures_.push_back(texture);
    }

    return textures;
}

bool Model::isTextureLoaded(std::string texture_path) const
{
    for (const auto& texture : loaded_textures_)
    {
        if (texture_path == texture.path)
            return true;
    }
    return false;
}

uint32_t TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    filename             = directory + '/' + filename;

    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    int            width, height, nr_components;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nr_components, 0);
    if (data)
    {
        GLenum format;
        if (nr_components == 1)
            format = GL_RED;
        else if (nr_components == 3)
            format = GL_RGB;
        else if (nr_components == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return texture_id;
}