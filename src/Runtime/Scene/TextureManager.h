#pragma once
// std include
#include <unordered_map>
#include <memory>
#include <string>
// thirdparty include

// Aurora include
#include "glWrapper/Texture.h"

namespace Aurora
{

class TextureManager
{
public:
    static TextureManager& GetInstance()
    {
        static TextureManager instance;
        return instance;
    }

    std::shared_ptr<Texture> GetTexture(const std::string& path);

private:
    TextureManager();

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_texture_map;
};
} // namespace Aurora