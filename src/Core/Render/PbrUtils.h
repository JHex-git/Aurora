#pragma once
// std include
#include <cstdint>

// Aurora include
#include "Runtime/Scene/Components/SubMesh.h"
#include "Runtime/Scene/TextureManager.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

struct PbrTextures
{
    TextureID base_color = 0;
    TextureID normal = 0;
    TextureID metalness = 0;
    TextureID roughness = 0;
    TextureID ambient_occlusion = 0;
    TextureID emissive = 0;
};

PbrTextures CollectPbrTextures(const SubMesh& submesh);

// Bind all PBR textures and set related uniforms on the given shader program.
// `unit` is advanced as textures are bound.
void BindPbrTextures(const SubMesh& submesh,
                     TextureManager& texture_manager,
                     ShaderProgram& shader_program,
                     int& unit);

} // namespace Aurora
