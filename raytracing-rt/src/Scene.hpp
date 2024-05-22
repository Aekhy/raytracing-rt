#pragma once

#include <vector>
#include <string>
#include "Material.hpp"
#include "Sphere.hpp"

struct Scene
{
    std::vector<Sphere> Spheres;
    std::vector<Material> Materials;

    void AddSphere(const glm::vec3& position, float radius, int materialIndex);
    void AddSphere(const Sphere& sphere);
    void saveScene(const std::string& filename) const;
    void loadScene(const std::string& filename);
};
