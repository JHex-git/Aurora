#pragma once
// std include
#include <cstdint>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"

namespace Aurora
{

struct LightUBOData
{
    static constexpr int kMaxLights = 4;

    struct alignas(16) Light
    {
        alignas(16) glm::vec3 light_pos;
        alignas(16) glm::vec3 light_color;
        alignas(16) glm::vec2 cull_distance; // x: near, y: far
        float light_intensity;
    } lights[kMaxLights];

    alignas(16) unsigned int num_lights = 0;
};

} // namespace Aurora
