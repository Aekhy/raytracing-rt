#pragma once

#include <glm/glm.hpp>
#include "include/json.hpp"

struct Sphere
{
    glm::vec3 Position{ 0.0f };
    float Radius = 0.5f;
    int MaterialIndex = 0;
};

void to_json(nlohmann::json& j, const Sphere& s);
void from_json(const nlohmann::json& j, Sphere& s);
