#pragma once
// std include

// thirdparty include

// Aurora include


namespace Aurora
{

class GlobalContext
{
public:
    static GlobalContext& GetInstance()
    {
        static GlobalContext context;
        return context;
    }

    bool draw_spatial_hierarchy = false;
    bool draw_directional_cascades = false;

private:
    GlobalContext() = default;
    ~GlobalContext() = default;
};
} // namespace Aurora
