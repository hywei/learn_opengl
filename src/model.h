#pragma once

#include <string>
#include <vector>

#include "mesh.h"

class Shader;
struct aiNode;
struct aiScene;
struct aiMaterial;
struct aiMesh;

class Model {

public:
    Model(const char* path);
    ~Model();

    void Draw(Shader& shader);

private:
    // model data
    std::vector<Mesh*>   meshes_;
    std::string          directory_;
    std::vector<Texture> loaded_textures_; // all textures loaded so far

    void  loadModel(std::string path);
    void  processNode(aiNode* node, const aiScene* scene);
    Mesh* processMesh(aiMesh* mesh, const aiScene* scene);

    // check all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture>
    loadMaterialTextures(aiMaterial* mat, uint32_t ai_texture_type, TextureType texture_type);

    bool isTextureLoaded(std::string texture_path) const;
};