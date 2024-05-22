#pragma once

#include "Material.hpp"
#include "Sphere.hpp"
#include "include/json.hpp"

void to_json(nlohmann::json& j, const Material& m);
void from_json(const nlohmann::json& j, Material& m);
void to_json(nlohmann::json& j, const Sphere& s);
void from_json(const nlohmann::json& j, Sphere& s);
