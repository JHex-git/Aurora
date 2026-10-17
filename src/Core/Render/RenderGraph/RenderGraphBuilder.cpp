// std include
#include <algorithm>
#include <cassert>
#include <set>
#include <limits>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Core/Render/RenderGraph/RenderGraphNode.h"
#include "Core/Render/RenderResourcePool.h"
#include "glWrapper/RenderEventInfo.h"

namespace Aurora
{

namespace
{
constexpr uint32_t k_invalid_pass_order = std::numeric_limits<uint32_t>::max();
}

void RenderGraphBuilder::Compile()
{
    m_is_compiled = false;
    m_sorted_passes.clear();

    // Build a map from resource ID to the pass that produces it.
    constexpr uint32_t INVALID_PASS_ID = std::numeric_limits<uint32_t>::max();
    std::vector<uint32_t> resource_producer_map(m_resources.size(), INVALID_PASS_ID);
    for (uint32_t pass_id = 0; pass_id < m_passes.size(); ++pass_id)
    {
        const auto& pass = m_passes[pass_id];
        for (const auto& resource_handle : pass->m_produced_resources)
        {
            assert(resource_producer_map[resource_handle.id] == INVALID_PASS_ID && "Resource is produced by multiple passes.");
            auto& resource = m_resources[resource_handle.id];
            resource_producer_map[resource_handle.id] = pass_id;
        }
    }

    // Build pass dependencies based on resource consumption
    std::vector<std::set<uint32_t>> pass_successors(m_passes.size());
    // TODO: Read After write caused by alias
    for (uint32_t pass_id = 0; pass_id < m_passes.size(); ++pass_id)
    {
        const auto& pass = m_passes[pass_id];
        for (const auto& resource_handle : pass->m_consumed_resources)
        {
            uint32_t producer_pass_id = resource_producer_map[resource_handle.id];
            assert(producer_pass_id != INVALID_PASS_ID && "Resource is consumed but not produced by any pass.");
            pass_successors[producer_pass_id].insert(pass_id);
        }
    }


    std::vector<uint32_t> in_degree(m_passes.size(), 0);
    for (const auto& successors : pass_successors)
    {
        for (uint32_t successor : successors)
        {
            in_degree[successor]++;
        }
    }

    // Topologically sort (Kahn's algorithm)
    m_sorted_passes.reserve(m_passes.size());
    for (uint32_t pass_id = 0; pass_id < m_passes.size(); ++pass_id)
    {
        if (in_degree[pass_id] == 0)
        {
            m_sorted_passes.push_back(pass_id);
        }
    }
    for (int pass_id = 0; pass_id < m_sorted_passes.size(); ++pass_id)
    {
        auto sorted_pass = m_sorted_passes[pass_id];
        for (uint32_t successor : pass_successors[sorted_pass])
        {
            in_degree[successor]--;
            if (in_degree[successor] == 0)
            {
                m_sorted_passes.push_back(successor);
            }
        }
    }

    // Cycle detection
    if (m_sorted_passes.size() != m_passes.size())
    {
        std::vector<bool> visited(m_passes.size(), false);
        for (uint32_t pass : m_sorted_passes)
        {
            visited[pass] = true;
        }
        for (uint32_t pass_id = 0; pass_id < m_passes.size(); ++pass_id)
        {
            if (!visited[pass_id])
            {
                spdlog::error("Cyclic dependency detected in render graph passes. Pass '{}' is part of the cycle.", m_passes[pass_id]->m_name);
            }
        }
        return;
    }

    // Calculate lifetimes for each resource based on the sorted passes
    const auto extend_lifetime = [](RenderGraphResource& resource, uint32_t pass_id)
    {
        if (resource.m_begin_pass_order == k_invalid_pass_order || pass_id < resource.m_begin_pass_order)
            resource.m_begin_pass_order = pass_id;
        if (resource.m_end_pass_order == k_invalid_pass_order || pass_id > resource.m_end_pass_order)
            resource.m_end_pass_order = pass_id;
    };

    for (uint32_t pass_id = 0; pass_id < m_sorted_passes.size(); ++pass_id)
    {
        auto sorted_pass = m_sorted_passes[pass_id];
        auto& pass = m_passes[sorted_pass];
        for (const auto& resource_handle : pass->m_produced_resources)
        {
            extend_lifetime(*m_resources[resource_handle.id], pass_id);
        }
        for (const auto& resource_handle : pass->m_consumed_resources)
        {
            extend_lifetime(*m_resources[resource_handle.id], pass_id);
        }
    }

    // Aliased versions share one physical allocation, so their lifetimes collapse onto the
    // chain root. Without this the root would be returned to the pool while a later
    // version still points at it.
    for (const auto& resource : m_resources)
    {
        if (resource == nullptr || !resource->IsAlias()) continue;
        extend_lifetime(*m_resources[resource->GetAliasRoot()], resource->m_begin_pass_order);
        extend_lifetime(*m_resources[resource->GetAliasRoot()], resource->m_end_pass_order);
    }
    for (const auto& resource : m_resources)
    {
        if (resource == nullptr || !resource->IsAlias()) continue;

        const auto& root = m_resources[resource->GetAliasRoot()];
        resource->m_begin_pass_order = root->m_begin_pass_order;
        resource->m_end_pass_order = root->m_end_pass_order;
    }

    m_release_by_order.assign(m_sorted_passes.size(), {});
    m_aliases_by_resource.assign(m_resources.size(), {});
    for (uint32_t resource_id = 0; resource_id < m_resources.size(); ++resource_id)
    {
        const auto& resource = m_resources[resource_id];
        if (resource == nullptr) continue;

        if (resource->IsAlias())
        {
            m_aliases_by_resource[resource->GetAliasRoot()].push_back(resource_id);
            continue;
        }

        if (resource->IsExternal() || resource->m_end_pass_order == k_invalid_pass_order) continue;
        m_release_by_order[resource->m_end_pass_order].push_back(resource_id);
    }

    // Cull not used passes
    m_alive_resources.assign(m_resources.size(), false);
    for (const auto& resource : m_resources)
    {
        if (resource->m_is_external) m_alive_resources[resource->m_id] = true;
    }
    m_alive_passes.assign(m_passes.size(), true);
    for (uint32_t pass_id = m_sorted_passes.size(); pass_id-- > 0;)
    {
        auto sorted_pass = m_sorted_passes[pass_id];
        auto& pass = m_passes[sorted_pass];
        bool is_alive = false;
        for (const auto& resource_handle : pass->m_produced_resources)
        {
            // An alias shares the memory of its chain root, so it is alive exactly when the
            // root is. Without this, overlays that render into an external target would be
            // culled because the alias itself was never marked as a sink.
            const auto& resource = m_resources[resource_handle.id];
            const uint32_t alive_id = resource->IsAlias() ? resource->GetAliasRoot() : resource_handle.id;
            if (m_alive_resources[resource_handle.id] || m_alive_resources[alive_id])
            {
                is_alive = true;
                break;
            }
        }
        if (!is_alive)
        {
            m_alive_passes[sorted_pass] = false;
            spdlog::info("Culling unused pass '{}'", pass->m_name);
            continue;
        }
        for (const auto& resource_handle : pass->m_consumed_resources)
        {
            m_alive_resources[resource_handle.id] = true;
        }
    }
    m_is_compiled = true;
}

void RenderGraphBuilder::Execute(ContextState& context_state, const std::array<int, 2>& viewport_size)
{
    if (!m_is_compiled) return;
    m_is_executing = true;

    RenderGraphContext context(context_state, *m_resource_pool, m_resources, viewport_size);

    for (uint32_t order = 0; order < m_sorted_passes.size(); ++order)
    {
        const uint32_t pass_id = m_sorted_passes[order];
        if (!m_alive_passes[pass_id]) continue;
        auto& pass = m_passes[pass_id];
        for (const auto& resource_handle : pass->m_produced_resources)
        {
            auto* resource = m_resources[resource_handle.id].get();
            switch (resource->GetType())
            {
            case RenderGraphResource::Type::Texture:
            {
                auto* texture = static_cast<RenderGraphTexture*>(resource);
                if (texture->GetResolvedTexture() != nullptr) break;

                // External resources are registered with their texture already resolved
                if (resource->IsExternal())
                {
                    spdlog::error("RenderGraphBuilder::Execute: external resource '{}' has no texture", texture->GetName());
                    assert(false && "external resource must be resolved at registration");
                    break;
                }

                // An aliased version reuses the memory of its chain root, which a previous
                // pass has already resolved. Only the root takes a texture out of the pool.
                if (texture->IsAlias())
                {
                    auto& root = m_resources[texture->GetAliasRoot()];
                    assert(root->GetType() == RenderGraphResource::Type::Texture);
                    auto* root_texture = static_cast<RenderGraphTexture*>(root.get());
                    if (root_texture->GetResolvedTexture() != nullptr)
                    {
                        texture->Resolve(root_texture->GetResolvedTexture());
                        break;
                    }
                    spdlog::warn("RenderGraphBuilder::Execute: alias root of '{}' is unresolved, allocating a new texture", texture->GetName());
                }

                texture->Resolve(m_resource_pool->AcquireTexture(texture->GetDescription()));
                break;
            }
            default:
                assert(false && "Execute cannot resolve this resource type yet");
                break;
            }
        }
        // Execute the pass here
        {
            SCOPED_RENDER_EVENT(pass->m_name);
            pass->m_execute(context);
        }

        // Release resources that are no longer needed
        for (uint32_t released_id : m_release_by_order[order])
        {
            auto* resource = m_resources[released_id].get();
            switch (resource->GetType())
            {
            case RenderGraphResource::Type::Texture:
            {
                auto* texture = static_cast<RenderGraphTexture*>(resource);
                m_resource_pool->ReleaseTexture(texture->GetResolvedTexture());
                texture->Resolve(nullptr);
                for (uint32_t alias_id : m_aliases_by_resource[released_id])
                {
                    auto* alias = static_cast<RenderGraphTexture*>(m_resources[alias_id].get());
                    alias->Resolve(nullptr);
                }
                break;
            }
            default:
                assert(false && "Execute cannot release this resource type yet");
                break;
            }
        }
    }
    m_is_executing = false;
}

RenderGraphTextureHandle RenderGraphBuilder::CreateTexture(std::string name, TextureDescription && description)
{
    assert(!m_is_executing, "resource creation should not happen during graph execution");
    const uint32_t id = static_cast<uint32_t>(m_resources.size());
    auto texture = std::make_unique<RenderGraphTexture>(id, std::move(name), std::move(description));
    m_resources.push_back(std::move(texture));
    return RenderGraphTextureHandle{ id };
}

RenderGraphTextureHandle RenderGraphBuilder::CreateAliasedTexture(std::string name, RenderGraphTextureHandle target)
{
    assert(!m_is_executing, "resource creation should not happen during graph execution");
    auto* source = m_resources[target.id].get();
    assert(source != nullptr && source->GetType() == RenderGraphResource::Type::Texture);
    if (source->HasAlias())
    {
        spdlog::error("Multiple alias created from one resource handle {}", target.id);
        return {};
    }
    auto* source_texture = static_cast<RenderGraphTexture*>(source);

    const uint32_t id = static_cast<uint32_t>(m_resources.size());
    auto texture = std::make_unique<RenderGraphTexture>(id, std::move(name), TextureDescription(source_texture->GetDescription()));
    texture->m_alias_root = source_texture->IsAlias() ? source_texture->GetAliasRoot() : source_texture->GetID();
    source->m_has_been_aliased = true;
    m_resources.push_back(std::move(texture));
    return RenderGraphTextureHandle{ id };
}

RenderGraphTextureHandle RenderGraphBuilder::RegisterExternalTexture(std::string name, Texture* texture)
{
    assert(!m_is_executing, "resource creation should not happen during graph execution");
    assert(texture != nullptr && "RegisterExternalTexture requires a valid texture");

    const uint32_t id = static_cast<uint32_t>(m_resources.size());
    auto render_graph_texture = std::make_unique<RenderGraphTexture>(id, std::move(name), TextureDescription(texture->GetDescription()), true);
    render_graph_texture->Resolve(texture);
    m_resources.push_back(std::move(render_graph_texture));
    return RenderGraphTextureHandle{ id };
}

}// namespace Aurora