#pragma once

namespace Aurora
{
    
class ILightEventListener
{
public:
    virtual ~ILightEventListener() = default;
    virtual void OnLightChanged() = 0;
};

} // namespace Aurora