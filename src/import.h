#pragma once

#include <string>
#include <vector>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//Broken class, needs fixing
class AssetManager
{
public:
    std::vector<std::string> asset_mesh_name;
    std::vector<const aiScene*> asset_mesh_value;
    std::vector<std::shared_ptr<Assimp::Importer>> asset_model_importers;
    
    AssetManager();
    void load_model(std::string path, std::string name, unsigned int import_flags);
    void load_audio(std::string path, std::string name);
    const void* get_asset(std::string name);
};