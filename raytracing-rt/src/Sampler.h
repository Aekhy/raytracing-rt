#pragma once
#include <random>
#include <glm/glm.hpp> // Include for glm::vec2
#include "Scene.hpp"

namespace CustomRand {
	extern thread_local std::mt19937 mersenneTwister;
	extern thread_local std::uniform_real_distribution<double> uniform;
	#define RND (2.0*uniform(mersenneTwister)-1.0)
	#define RND2 (uniform(mersenneTwister))

	static float uniform_random_value() {
		return RND2;
	}
}

class Sampler 
{
public:
	// get omega and brdf multiplier
	glm::vec3 sample(Material material, glm::vec3 normal, glm::vec3 omega) const;

	// Helpers for random number generation
	glm::vec3 cosine_weighted_hemisphere() const;
	glm::vec3 local_to_world(const glm::vec3& localDir, const glm::vec3& n) const;
private:
	// get brdf
	glm::vec3 eval(Material material) const;
	// get probability of sample
	float pdf(Material material) const;

	void ons(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) const;

	
};