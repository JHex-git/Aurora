// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/LightManager.h"

namespace Aurora
{

LightID LightManager::RegisterLight(std::shared_ptr<Light> light)
{
    static LightID light_id = 1;
    if (light)
    {
        m_lights[static_cast<size_t>(light->GetType())].emplace(light_id, light);
        NotifyLightEventListeners();
        return light_id++;
    }
    return Light::INVALID_LIGHT_ID;
}

void LightManager::UnregisterLight(LightID lightID)
{
    for (auto& light_map : m_lights)
    {
        light_map.erase(lightID);
    }
    NotifyLightEventListeners();
}

void LightManager::AddLightEventListener(ILightEventListener* listener)
{
    if (listener)
    {
        m_light_event_listeners.push_back(listener);
    }
}

void LightManager::RemoveLightEventListener(ILightEventListener* listener)
{
    if (listener)
    {
        m_light_event_listeners.erase(std::remove(m_light_event_listeners.begin(), m_light_event_listeners.end(), listener), m_light_event_listeners.end());
    }
}

void LightManager::NotifyLightEventListeners() const
{
    for (ILightEventListener* listener : m_light_event_listeners)
    {
        listener->OnLightChanged();
    }
}

} // namespace Aurora