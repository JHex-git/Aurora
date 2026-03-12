// std include

// Aurora include
#include "Core/Render/PbrUtils.h"

namespace Aurora
{

PbrTextures CollectPbrTextures(const SubMesh& submesh)
{
    PbrTextures textures;
    for (const auto texture_id : submesh.GetTextures())
    {
        auto& surface_texture = TextureManager::GetInstance().GetTexture(texture_id);
        const auto& type = surface_texture.type;
        if (type == "BaseColor")
        {
            textures.base_color = texture_id;
        }
        else if (type == "Diffuse" && textures.base_color == 0)
        {
            textures.base_color = texture_id;
        }
        else if (type == "Normal" || type == "Normals")
        {
            textures.normal = texture_id;
        }
        else if (type == "Metalness")
        {
            textures.metalness = texture_id;
        }
        else if (type == "DiffuseRoughness")
        {
            textures.roughness = texture_id;
        }
        else if (type == "AmbientOcclusion")
        {
            textures.ambient_occlusion = texture_id;
        }
        else if (type == "Emissive" || type == "EmissionColor")
        {
            textures.emissive = texture_id;
        }
    }

    return textures;
}

void BindPbrTextures(const SubMesh& submesh,
                     TextureManager& texture_manager,
                     ShaderProgram& shader_program,
                     int& unit)
{
    const auto pbr_textures = CollectPbrTextures(submesh);
    auto& dummy_white = texture_manager.GetDummyWhiteTexture();
    auto& dummy_black = texture_manager.GetDummyBlackTexture();
    auto& dummy_normal = texture_manager.GetDummyNormalTexture();

    shader_program.SetUniform("uBaseColorFactor", submesh.GetColor());
    shader_program.SetUniform("uMetallicFactor", pbr_textures.metalness != 0 ? 1.0f : 0.0f);
    shader_program.SetUniform("uRoughnessFactor", 1.0f);
    shader_program.SetUniform("uEmissiveFactor", glm::vec3(1.0f));

    shader_program.SetUniform("uHasBaseColorMap", pbr_textures.base_color != 0);
    {
        auto& surface_texture = pbr_textures.base_color != 0
            ? texture_manager.GetTexture(pbr_textures.base_color)
            : dummy_white;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexBaseColor", unit - 1);
    }

    shader_program.SetUniform("uHasNormalMap", pbr_textures.normal != 0);
    {
        auto& surface_texture = pbr_textures.normal != 0
            ? texture_manager.GetTexture(pbr_textures.normal)
            : dummy_normal;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexNormal", unit - 1);
    }

    shader_program.SetUniform("uHasMetalnessMap", pbr_textures.metalness != 0);
    {
        auto& surface_texture = pbr_textures.metalness != 0
            ? texture_manager.GetTexture(pbr_textures.metalness)
            : dummy_black;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexMetalness", unit - 1);
    }

    shader_program.SetUniform("uHasRoughnessMap", pbr_textures.roughness != 0);
    {
        auto& surface_texture = pbr_textures.roughness != 0
            ? texture_manager.GetTexture(pbr_textures.roughness)
            : dummy_white;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexRoughness", unit - 1);
    }

    shader_program.SetUniform("uHasAoMap", pbr_textures.ambient_occlusion != 0);
    {
        auto& surface_texture = pbr_textures.ambient_occlusion != 0
            ? texture_manager.GetTexture(pbr_textures.ambient_occlusion)
            : dummy_white;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexAmbientOcclusion", unit - 1);
    }

    shader_program.SetUniform("uHasEmissiveMap", pbr_textures.emissive != 0);
    {
        auto& surface_texture = pbr_textures.emissive != 0
            ? texture_manager.GetTexture(pbr_textures.emissive)
            : dummy_black;
        surface_texture.texture.Bind(unit++);
        shader_program.SetUniform("uTexEmissive", unit - 1);
    }
}

} // namespace Aurora
