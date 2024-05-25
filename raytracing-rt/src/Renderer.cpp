#include "Camera.h"
#include "Renderer.h"
#include "Scene.hpp"
#include "Walnut/Random.h"
#include <execution>
#include <glm/gtx/component_wise.hpp>
#include "Sampler.h"
#include "MyRand.h"

#define eps 0.0001f
#define M_PI 3.14159265358979323846  /* pi */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

namespace Utils {

	static uint32_t ConvertToRGBA(const glm::vec4 color)
	{
		uint8_t r = (color.r * 255.0f);
		uint8_t g = (color.g * 255.0f);
		uint8_t b = (color.b * 255.0f);
		uint8_t a = (color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height) {

	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else 
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);

	for (uint32_t i = 0; i < width; ++i)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < height; ++i)
		m_ImageVerticalIter[i] = i;

}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	const int N_MC = GetSettings().MonteCarloNbSample;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	// prepare some random offset for antialiasing
	if (GetSettings().Antialiasing) 
	{
		m_AntialiasingOffset.resize(GetSettings().MonteCarloNbSample);
		for (int i = 0; i < N_MC; ++i) {
			m_AntialiasingOffset[i].x = CustomRand::uniform_random_value() - 0.5f;
			m_AntialiasingOffset[i].y = CustomRand::uniform_random_value() - 0.5f;
		}
	}
	
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this, N_MC](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
			[this, y, N_MC](uint32_t x)
				{
					int index = x + y * m_FinalImage->GetWidth();

					//glm::vec4 color = PerPixel(x, y);


					// monte carlo
					glm::vec3 radiance{0};
					for (int i = 0; i < N_MC; ++i)
					{
						Ray ray;
						ray.Origin = m_ActiveCamera->GetPosition();
						ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

						if (GetSettings().Antialiasing)
						{
							// Generate small random offsets for anti-aliasing
							// avoid for randering new offset point for each pixel, it's new at each frame
							float offsetX = m_AntialiasingOffset[i].x / m_ActiveCamera->GetViewportWidth();
							float offsetY = m_AntialiasingOffset[i].y / m_ActiveCamera->GetViewportHeight();
							ray.Direction += offsetX * glm::cross(m_ActiveCamera->GetDirection(), glm::vec3(0.0f, 1.0f, 0.0f)) + offsetY * glm::vec3(0.0f, 1.0f, 0.0f);
						}
						
						radiance += Li(ray, 0, glm::vec3{1.0f});
					}
					radiance /= N_MC;
					
					glm::vec4 color(radiance, 1);
					m_AccumulationData[index] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[index];
					accumulatedColor /= (float)m_FrameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[index] = Utils::ConvertToRGBA(accumulatedColor);

				});
		});
	/*
	float Lavg = 0.0f;
	
	for (auto y = 0; y < m_FinalImage->GetHeight(); ++y) {
		for (auto x = 0; x < m_FinalImage->GetWidth(); ++x) {
			int index = x + y * m_FinalImage->GetWidth();

			glm::vec4 accumulatedColor = m_AccumulationData[index];
			accumulatedColor /= (float)m_FrameIndex;

			Lavg += log()
		}
	}
	*/
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
	{
		m_FrameIndex++;
	}
	else
	{
		m_FrameIndex = 1;
	}
}


glm::vec3 Renderer::Li(Ray ray, int bounce, glm::vec3 throughput) {
	// no russian roulette
	//if (bounce > 10) return glm::vec3(0);

	HitPayload payload = TraceRay(ray);
	
	if (payload.HitDistance < eps) {
		return glm::vec3(0.5f, 0.6f, 0.8f);
		return glm::vec3{ 0 };
	}

	const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
	const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

	glm::vec3 radiance = material.GetEmission();

	const int MIN_BOUNCES = 3;
	float rr_prob;
	if (bounce < MIN_BOUNCES) {
		rr_prob = 1.0f;
	}
	else {
		rr_prob = MAX(MAX(throughput.x, throughput.y), throughput.z);
		rr_prob = glm::clamp(rr_prob, 0.0f, 0.99f);
	}

	if (CustomRand::uniform_random_value() >= rr_prob) return radiance;

	
	Ray newRay;
	newRay.Origin = payload.WorldPosition; // +0.0001f * payload.WorldNormal;
	glm::vec3 brdfmultiplier = sampler.sample(ray.Direction, material, payload.WorldNormal, newRay.Direction);
	throughput *= brdfmultiplier / rr_prob;
	radiance += brdfmultiplier * Li(newRay, bounce + 1, throughput) / rr_prob;

	return radiance;
}



glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 light(0.0f);
	// placeholder to not get something too bright
	glm::vec3 contribution{ 1.0f };

	int bounces = 2000;
	for (int i = 0; i < bounces; ++i)
	{
		// object intersection
		Renderer::HitPayload payload = TraceRay(ray);

		// Background color
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = glm::vec3(0.5f, 0.6f, 0.8f);
			light += skyColor * contribution;
			break;
		}
			
		// lighting
		//glm::vec3 lightDirection = glm::normalize(glm::vec3(-1, -1, -1));
		//float d = glm::max(glm::dot(payload.WorldNormal, -lightDirection), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		contribution *= material.Albedo;
		light += material.GetEmission() * material.Albedo;

		ray.Origin = payload.WorldPosition; //+ 0.0001f * payload.WorldNormal;

		ray.Direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
	
	}

	return glm::vec4(light, 1);

}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); ++i)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 Origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(Origin, ray.Direction);
		float c = glm::dot(Origin, Origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		if (t1 < eps)
		{
			t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
			if (t1 < eps) {
				continue;
			}
		}

		if (t1 < hitDistance)
		{
			hitDistance = t1;
			closestSphere = (int)i;
		}
	}
	// Miss
	if (closestSphere < 0 )
		return Miss(ray);
	
	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 Origin = ray.Origin - closestSphere.Position; // offset to origin for calcul
	payload.WorldPosition = Origin + hitDistance * ray.Direction;
	payload.WorldNormal = glm::normalize(payload.WorldPosition); // come back to original position
	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}