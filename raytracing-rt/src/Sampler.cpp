#include "Sampler.h"
#include "MyRand.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp> 

#define M_PI 3.14159265358979323846  /* pi */


bool vectorsAreEqual(const glm::vec3& v1, const glm::vec3& v2, float epsilon) {
	return (fabs(v1.x - v2.x) < epsilon) &&
		(fabs(v1.y - v2.y) < epsilon) &&
		(fabs(v1.z - v2.z) < epsilon);
}

glm::vec3 Sampler::sample(glm::vec3 incomingOmega, Material material, glm::vec3 normal, glm::vec3& omega) const
{
	if (material.Type == DIFFUSE) {
		
		// Sample new direction on the hemisphere
		glm::vec3 localDir = cosine_weighted_hemisphere();
		glm::vec3 worldDir = local_to_world(localDir, normal);

		omega = worldDir;// glm::normalize(worldDir + normal);
		return  material.Albedo; //eval(material) / pdf(material); Pi canel out
	}
	else if (material.Type == METALLIC) 
	{

		omega = glm::reflect(incomingOmega, normal);
		return  material.Albedo;
	} 
	else if (material.Type == DIELECTRIC)
	{
		float nt = material.IndiceIn;
		float ni = material.IndiceOut;
		float R0 = (ni - nt) / (ni + nt);
		R0 = R0 * R0;

		glm::vec3 N = normal;

		if (glm::dot(N, incomingOmega) > 0) // from inside
		{
			N = N * -1.0f;
			// swap ni, nt
			float tmp = nt;
			nt = ni;
			ni = tmp;
		}
		float n = ni / nt;
		float cosin = glm::dot(N, incomingOmega) * -1.0f;
		float x = (1.0f - cosin);
		float ReflProb = R0 + (1.0f - R0) * x * x * x * x * x;
		float cost2 = 1.0f - n * n * (1.0f - cosin * cosin);

		float rdmChoice = CustomRand::uniform_random_value();
		if (cost2 > 0 && rdmChoice > ReflProb) // refraction
		{
			glm::vec3 refracted = glm::normalize((incomingOmega * n) + (N * (n * cosin - sqrt(cost2))));
			omega = refracted;

			glm::vec3 delta_direction = glm::normalize(incomingOmega) - omega;
			return glm::vec3(1.10f);
			
		}
		else
		{
			omega = glm::normalize((incomingOmega + N * (cosin * 2)));
			return glm::vec3(ReflProb);;

		}

	}
}

glm::vec3 Sampler::eval(Material material) const
{
	if (material.Type == DIFFUSE) {
		return material.Albedo /(float) M_PI;
	}
	else if (material.Type == METALLIC) {
		return glm::vec3{ 0 };
	}
}

float Sampler::pdf(Material material) const
{
	if (material.Type == DIFFUSE) {
		return 1.0f / (float)M_PI;
	}
	else if (material.Type == METALLIC) {
		return 0.0f;
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

