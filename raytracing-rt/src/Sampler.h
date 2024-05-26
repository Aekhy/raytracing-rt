#pragma once
#include <glm/glm.hpp> // Include for glm::vec2
#include "Scene.hpp"


class Sampler 
{
public:
	// get omega and brdf multiplier
	glm::vec3 sample(glm::vec3 incomingOmega, Material material, glm::vec3 normal, glm::vec3& omega) const;

	// Helpers for random number generation
	glm::vec3 cosine_weighted_hemisphere() const;
	glm::vec3 local_to_world(const glm::vec3& localDir, const glm::vec3& n) const;
private:
	// get brdf
	glm::vec3 eval(glm::vec3 incomingOmega, Material material, glm::vec3 normal, glm::vec3 omega) const;
	// get probability of sample
	float pdf(glm::vec3 incomingOmega, Material material, glm::vec3 normal, glm::vec3 omega) const;

	void ons(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) const;

	
};