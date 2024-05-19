#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{
	char* Name = "White Default";
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor{ 0.0f };
	float EmssionPower = 0.0f;

	glm::vec3 GetEmssision() const { return EmssionPower * EmissionColor; }
};


struct Sphere
{
	glm::vec3 Position{ 0.0f};
	float Radius = 0.5f;
	int MaterialIndex = 0;;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;


	void AddSphere(const glm::vec3& position, float radius, int materialIndex) 
	{
		AddSphere(Sphere{ position, radius, materialIndex });
	}
	void AddSphere(const Sphere& sphere) {
		Spheres.push_back(sphere);
	}
};