#pragma once
// std include
#include <vector>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Runtime/Scene/SceneObjects/Lights/Light.h"

namespace Aurora
{

class LightManager
{
public:
    static LightManager& GetInstance()
    {
        static LightManager instance;
        return instance;
    }

    void AddLight(Light&& light);
    void RemoveLight();
    const Light& GetLight(const glm::vec3& viewPos);
    
private:
    LightManager() = default;

    std::vector<Light> m_lights;
};
} // namespace Aurora