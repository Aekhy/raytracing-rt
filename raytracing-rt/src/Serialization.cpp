#include "Serialization.hpp"

void to_json(nlohmann::json& j, const Material& m) {
    j = nlohmann::json{
        {"Name", m.Name},
        {"Albedo", {m.Albedo.x, m.Albedo.y, m.Albedo.z}},
        {"Roughness", m.Roughness},
        {"Metallic", m.Metallic},
        {"EmissionColor", {m.EmissionColor.x, m.EmissionColor.y, m.EmissionColor.z}},
        {"EmssionPower", m.EmssionPower}
    };
}

void from_json(const nlohmann::json& j, Material& m) {
    std::string name = j.at("Name").get<std::string>();
    m.Name = new char[name.size() + 1];  // Allocate memory for Name
    std::copy(name.begin(), name.end(), m.Name);
    m.Name[name.size()] = '\0';  // Null-terminate the string
    auto albedo = j.at("Albedo").get<std::vector<float>>();
    m.Albedo = glm::vec3(albedo[0], albedo[1], albedo[2]);
    m.Roughness = j.at("Roughness").get<float>();
    m.Metallic = j.at("Metallic").get<float>();
    auto emissionColor = j.at("EmissionColor").get<std::vector<float>>();
    m.EmissionColor = glm::vec3(emissionColor[0], emissionColor[1], emissionColor[2]);
    m.EmssionPower = j.at("EmssionPower").get<float>();
}

void to_json(nlohmann::json& j, const Sphere& s) {
    j = nlohmann::json{
        {"Position", {s.Position.x, s.Position.y, s.Position.z}},
        {"Radius", s.Radius},
        {"MaterialIndex", s.MaterialIndex}
    };
}

void from_json(const nlohmann::json& j, Sphere& s) {
    auto position = j.at("Position").get<std::vector<float>>();
    s.Position = glm::vec3(position[0], position[1], position[2]);
    s.Radius = j.at("Radius").get<float>();
    s.MaterialIndex = j.at("MaterialIndex").get<int>();
}
