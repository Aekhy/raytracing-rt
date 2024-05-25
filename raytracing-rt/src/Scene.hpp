#pragma once

#include <vector>
#include <string>
#include "Material.hpp"
#include "Sphere.hpp"

struct Scene
{
    std::vector<Sphere> Spheres;
    std::vector<Material> Materials;

    void Scene::AddMaterial(char* Name,
        glm::vec3 Albedo,
        float Roughness,
        float Metallic,
        glm::vec3 EmissionColor,
        float EmissionPower,
        MaterialType Type,
        float IndiceOut,
        float IndiceIn);
    void AddSphere(const glm::vec3& position, float radius, int materialIndex);
    void AddSphere(const Sphere& sphere);
    void saveScene(const std::string& filename) const;
    void loadScene(const std::string& filename);
};
