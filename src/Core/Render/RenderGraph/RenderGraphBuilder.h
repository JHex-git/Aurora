#pragma once
// std include
#include <array>
#include <type_traits>
#include <vector>
#include <memory>
#include <string>
// thirdparty include
// Aurora include
#include "Core/Render/RenderGraph/RenderGraphTexture.h"

namespace Aurora
{
class ContextState;
class RenderResourcePool;

struct PassParameterBase
{
    virtual ~PassParameterBase() = default;
};

class RenderGraphBuilder
{
public:
    RenderGraphBuilder(RenderResourcePool* resource_pool) : m_resource_pool(resource_pool) {}
    ~RenderGraphBuilder() = default;

    template<typename SetupLambda, typename ExecuteLambda>
    void AddPass(std::string pass_name, SetupLambda&& setup, ExecuteLambda&& execute)
    {
        static_assert(std::is_invocable_v<SetupLambda, RenderGraphPass&>,
                      "setup must be invocable with (RenderGraphPass&)");

        auto pass = std::make_unique<RenderGraphPass>(pass_name);
        setup(*pass);
        pass->m_execute = std::move(execute);
        m_passes.push_back(std::move(pass));
    }

    void Compile();
    void Execute(ContextState& context_state, const std::array<int, 2>& viewport_size);

    RenderGraphTextureHandle CreateTexture(std::string name, TextureDescription&& description);

    // Creates a new logical target texture that reuses the memory of source texture.
    RenderGraphTextureHandle CreateAliasedTexture(std::string name, RenderGraphTextureHandle target);

    RenderGraphTextureHandle RegisterExternalTexture(std::string name, Texture* texture);

    template<typename ParameterType>
    ParameterType* AllocateParameter()
    {
        static_assert(std::is_base_of<PassParameterBase, ParameterType>::value, "ParameterType must derive from PassParameterBase");
        auto param = std::make_unique<ParameterType>();
        ParameterType* allocated = param.get();
        m_parameters.push_back(std::move(param));
        return allocated;
    }

private:
    std::vector<std::unique_ptr<RenderGraphResource>> m_resources;
    std::vector<std::unique_ptr<RenderGraphPass>> m_passes;
    std::vector<uint32_t> m_sorted_passes;
    std::vector<bool> m_alive_passes;
    std::vector<bool> m_alive_resources;
    std::vector<std::vector<uint32_t>> m_release_by_order;
    std::vector<std::vector<uint32_t>> m_aliases_by_resource;
    std::vector<std::unique_ptr<PassParameterBase>> m_parameters;
    RenderResourcePool* m_resource_pool = nullptr;
    bool m_is_compiled = false;
    bool m_is_executing = false;
};
} // namespace Aurora
