// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/LightManager.h"

namespace Aurora
{

LightID LightManager::RegisterLight(std::shared_ptr<Light> light)
{
    static LightID light_id = 1;
    m_lights.emplace(light_id, light);
    return light_id++;
}

void LightManager::UnregisterLight(LightID lightID)
{
    m_lights.erase(lightID);
}

void LightManager::UpdateActiveLights(const glm::vec3& viewPos)
{
    // TODO:
    m_active_lights.clear();
    for (const auto& light : m_lights)
    {
        m_active_lights.push_back(light.second);
    }
}

} // namespace Aurora