#pragma once

#pragma once

#include "Walnut/Image.h"  // Ensure this include is correct and the path is correct

#include "Camera.h"
#include "Ray.h"

#include <memory>  // Include for std::shared_ptr
#include <glm/glm.hpp> // Include for glm::vec2


class Renderer
{
public:
    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Camera& camera);

    std::shared_ptr<Walnut::Image> GetFinalImage() const {
        return m_FinalImage;
    }

private:
    glm::vec4 TraceRay(const Ray& ray);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
};


