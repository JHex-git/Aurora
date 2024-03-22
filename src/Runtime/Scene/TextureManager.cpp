// std include

// thirdparty include
#include "thirdparty/stb/stb_image.h"
// Aurora include
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

TextureManager::TextureManager()
{
    stbi_set_flip_vertically_on_load(true);
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& path)
{
    if (m_texture_map.find(path) == m_texture_map.end())
    {
        m_texture_map[path] = std::make_shared<Texture>();
        if (!m_texture_map[path]->Load(path, Texture::WrapType::ClampToEdge, Texture::WrapType::ClampToEdge))
        {
            m_texture_map.erase(path);
            return nullptr;
        }
    }
    return m_texture_map[path];
}
} // namespace Aurora