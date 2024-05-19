#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Sphere
{
	glm::vec3 Position{ 0.0f};
	float Radius = 0.5f;

	glm::vec3 Albedo{ 1.0f };
};

struct Scene
{
	std::vector<Sphere> Spheres;

	void AddSphere(const glm::vec3& position, float radius, const glm::vec3& albedo) 
	{
		AddSphere(Sphere{ position, radius, albedo });
	}
	void AddSphere(const Sphere& sphere) {
		Spheres.push_back(sphere);
	}
};