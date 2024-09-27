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

private:
    GlobalContext() = default;
    ~GlobalContext() = default;
};
} // namespace Aurora