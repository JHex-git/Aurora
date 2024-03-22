// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/LightManager.h"

namespace Aurora
{

void LightManager::AddLight(Light&& light)
{
    m_lights.push_back(std::move(light));
}

void LightManager::RemoveLight()
{
}

const Light& LightManager::GetLight(const glm::vec3& viewPos)
{
    // TODO:
    return m_lights[0];
}
} // namespace Aurora