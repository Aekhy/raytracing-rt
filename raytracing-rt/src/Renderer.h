#pragma once

#pragma once

#include "Walnut/Image.h"  // Ensure this include is correct and the path is correct

#include "Camera.h"
#include "Ray.h"
#include "Scene.hpp"
#include "Sampler.h"

#include <memory>  // Include for std::shared_ptr
#include <glm/glm.hpp> // Include for glm::vec2


class Renderer
{
public:
    struct Settings
    {
        bool Accumulate = true;
    };

    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Scene& scene, const Camera& camera);

    std::shared_ptr<Walnut::Image> GetFinalImage() const {
        return m_FinalImage;
    }

    void ResetFrameIndex() { m_FrameIndex = 1; }
    uint32_t GetFrameIndex() { return m_FrameIndex; };

    Settings& GetSettings() { return m_Settings; }

private:
    struct HitPayload
    {
        float HitDistance;
        glm::vec3 WorldPosition;
        glm::vec3 WorldNormal;

        int ObjectIndex;
    };


    glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen Shader
    glm::vec3 Li(Ray ray, int bounce, glm::vec3 throughput);
    HitPayload TraceRay(const Ray& ray);
    HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
    HitPayload Miss(const Ray& ray);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    Settings m_Settings;

    // iterator for multy threading
    std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;
    int NbMonteCarloSample;
    std::vector<int> iteratorMonteCarloSample;

    const Scene* m_ActiveScene = nullptr;
    const Camera* m_ActiveCamera = nullptr;

    glm::vec3 radiance;

    uint32_t* m_ImageData = nullptr;
    glm::vec4* m_AccumulationData = nullptr;

    uint32_t m_FrameIndex = 1;

    const Sampler sampler;
};


