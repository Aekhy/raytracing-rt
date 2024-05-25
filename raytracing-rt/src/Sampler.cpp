#include "Sampler.h"

#define M_PI 3.14159265358979323846  /* pi */

namespace CustomRand {
	std::mt19937 mersenneTwister;
	std::uniform_real_distribution<double> uniform(0.0, 1.0);  // Initialize distribution
}


glm::vec3 Sampler::sample(Material material, glm::vec3 normal, glm::vec3 omega) const
{
	if (material.Type == DIFFUSE) {
		
		// Sample new direction on the hemisphere
		glm::vec3 localDir = cosine_weighted_hemisphere();
		glm::vec3 worldDir = local_to_world(localDir, normal);

		float cosTheta = glm::dot(normal, worldDir);

		omega = worldDir;
		return cosTheta * eval(material) / pdf(material);
	}
}

glm::vec3 Sampler::eval(Material material) const
{
	if (material.Type == DIFFUSE) {
		return material.Albedo /(float) M_PI;
	}
}

float Sampler::pdf(Material material) const
{
	if (material.Type == DIFFUSE) {
		return 1.0f / (float)M_PI;
	}


}


void Sampler::ons(const glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) const {
	if (std::abs(v1.x) > std::abs(v1.y)) {
		// project to the y = 0 plane and construct a normalized orthogonal vector in this plane
		float invLen = 1.f / sqrtf(v1.x * v1.x + v1.z * v1.z);
		v2 = glm::vec3(-v1.z * invLen, 0.0f, v1.x * invLen);
	}
	else {
		// project to the x = 0 plane and construct a normalized orthogonal vector in this plane
		float invLen = 1.0f / sqrtf(v1.y * v1.y + v1.z * v1.z);
		v2 = glm::vec3(0.0f, v1.z * invLen, -v1.y * invLen);
	}
	v3 = glm::cross(v1, v2);
}

glm::vec3 Sampler::local_to_world(const glm::vec3& localDir, const glm::vec3& n) const {
	glm::vec3 t, b;
	ons(n, t, b);
	return localDir.x * t + localDir.y * b + localDir.z * n;
}

glm::vec3 Sampler::cosine_weighted_hemisphere() const {
	float u1 = CustomRand::uniform_random_value();
	float u2 = CustomRand::uniform_random_value();

	float r = sqrtf(u1);
	float theta = 2 * M_PI * u2;

	float x = r * cos(theta);
	float y = r * sin(theta);
	float z = sqrt(1.0f - u1); // z = cos(theta), but for u1 distribution

	return glm::vec3(x, y, z);
}

