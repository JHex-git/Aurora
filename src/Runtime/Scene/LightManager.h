#pragma once
// std include
#include <unordered_map>
#include <memory>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Runtime/Scene/Components/Light.h"

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

    LightID RegisterLight(std::shared_ptr<Light> light);
    void UnregisterLight(LightID lightID);
    const std::vector<std::shared_ptr<Light>>& GetActiveLights() { return m_active_lights; }
    void UpdateActiveLights(const glm::vec3& viewPos);
    
private:
    LightManager() = default;

    std::unordered_map<LightID, std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Light>> m_active_lights;
};
} // namespace Aurora