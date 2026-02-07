#pragma once
// std include
#include <unordered_map>
#include <memory>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
// Aurora include
#include "Runtime/Scene/Components/Light.h"
#include "Runtime/Scene/ILightEventListener.h"

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

    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    LightManager(LightManager&&) = delete;
    LightManager& operator=(LightManager&&) = delete;

    LightID RegisterLight(std::shared_ptr<Light> light);
    void UnregisterLight(LightID lightID);
    const std::unordered_map<LightID, std::weak_ptr<Light>>& GetLights(Light::Type type) { return m_lights[static_cast<size_t>(type)]; }

    void AddLightEventListener(ILightEventListener* listener);
    void RemoveLightEventListener(ILightEventListener* listener);
private:
    LightManager() = default;

    void NotifyLightEventListeners() const;

    std::array<std::unordered_map<LightID, std::weak_ptr<Light>>, static_cast<size_t>(Light::Type::MaxTypes)> m_lights;

    std::vector<ILightEventListener*> m_light_event_listeners;
};
} // namespace Aurora