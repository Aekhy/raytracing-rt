#include "Scene.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Serialization.hpp"
#include "include/stb_image.h"

void Scene::AddMaterial(char* Name,
glm::vec3 Albedo,
float Roughness,
float Metallic,
glm::vec3 EmissionColor,
float EmissionPower,
MaterialType Type,
float IndiceOut,
float IndiceIn)
{
    Materials.push_back(Material{ Name, Albedo, Roughness, Metallic, EmissionColor, EmissionPower, Type, IndiceOut, IndiceIn });
}

void Scene::AddSphere(const glm::vec3& position, float radius, int materialIndex)
{
    AddSphere(Sphere{ position, radius, materialIndex });
}

void Scene::AddSphere(const Sphere& sphere) {
    Spheres.push_back(sphere);
}

void Scene::saveScene(const std::string& filename) const {
    nlohmann::json j;
    j["Spheres"] = Spheres;
    j["Materials"] = Materials;

    // Construct the full path to the scenes folder
    std::filesystem::path scenesFolder = std::filesystem::current_path() / "scenes";
    std::filesystem::path fullPath = scenesFolder / filename;

    // Ensure the scenes directory exists
    if (!std::filesystem::exists(scenesFolder)) {
        std::filesystem::create_directories(scenesFolder);
    }

    std::ofstream file(fullPath);
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with 4 spaces of indentation
        file.close();
    }
    else {
        throw std::runtime_error("Could not open file for writing: " + fullPath.string());
    }
}

void Scene::loadCubemap(const char * filename)
{

    std::filesystem::path cubemapsFolder = std::filesystem::current_path() / "cubemaps";
    std::filesystem::path fullPath = cubemapsFolder / filename;

    // Ensure the scenes directory exists
    if (!std::filesystem::exists(cubemapsFolder)) {
        std::filesystem::create_directories(cubemapsFolder);
        Cubemap.exist = false;
        return ;
    }

    std::string fullPathStr = fullPath.string();
    const char* fullPathCStr = fullPathStr.c_str();


    int n, ok;
    ok = stbi_info(fullPathCStr, &Cubemap.width, &Cubemap.height, &Cubemap.nchannel);
    if (ok) {
        Cubemap.data = stbi_load(fullPathCStr, &Cubemap.width, &Cubemap.height, &Cubemap.nchannel, 0);
        Cubemap.exist = true;
    }
    else {
        Cubemap.exist = false;

        return;
    }
}

void Scene::loadScene(const std::string& filename) {

    // Construct the full path to the scenes folder
    std::filesystem::path scenesFolder = std::filesystem::current_path() / "scenes";
    std::filesystem::path fullPath = scenesFolder / filename;

    std::ifstream file(fullPath);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;

        Spheres = j.at("Spheres").get<std::vector<Sphere>>();
        Materials = j.at("Materials").get<std::vector<Material>>();

        file.close();
    }
    else {
        throw std::runtime_error("Could not open file for reading: " + fullPath.string());
    }
}
