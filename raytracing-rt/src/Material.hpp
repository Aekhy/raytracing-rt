#pragma once

#include <glm/glm.hpp>
#include "include/json.hpp"

enum MaterialType {
    DIFFUSE,
    METALLIC
};

struct Material
{
    char* Name = "White Default";
    glm::vec3 Albedo{ 1.0f };
    float Roughness = 1.0f;
    float Metallic = 0.0f;
    glm::vec3 EmissionColor{ 0.0f };
    float EmissionPower = 0.0f;
    MaterialType Type = DIFFUSE;

    glm::vec3 GetEmission() const { return EmissionPower * EmissionColor; }
};

void to_json(nlohmann::json& j, const Material& m);
void from_json(const nlohmann::json& j, Material& m);
