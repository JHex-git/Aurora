#pragma once
// std include
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace Aurora
{

class RenderGraphContext;

struct RenderGraphResourceHandle {
    static constexpr uint32_t kInvalidId = std::numeric_limits<uint32_t>::max();

    uint32_t id = kInvalidId;

    bool IsValid() const { return id != kInvalidId; }
};

class RenderGraphPass
{
    friend class RenderGraphBuilder;
public:
    RenderGraphPass(std::string name) : m_name(name) {}

    void Consume(RenderGraphResourceHandle resource) { m_consumed_resources.push_back(resource); }
    void Produce(RenderGraphResourceHandle resource) { m_produced_resources.push_back(resource); }
private:
    std::string m_name;
    std::vector<RenderGraphResourceHandle> m_consumed_resources;
    std::vector<RenderGraphResourceHandle> m_produced_resources;
    std::function<void(RenderGraphContext&)> m_execute;
};

class RenderGraphResource
{
    friend class RenderGraphBuilder;
public:
    RenderGraphResource(uint32_t id, bool is_external = false) : m_id(id), m_is_external(is_external) {}
    virtual ~RenderGraphResource() = default;

    enum class Type
    {
        Texture
    };

    uint32_t GetID() const { return m_id; }
    Type GetType() const { return m_type; }
    bool IsExternal() const { return m_is_external; }

    // Aliasing is a resource level concept: a later version can share the allocation of an
    // earlier one, so a pass can write it after reading the previous version while every
    // version still has exactly one producer.
    bool IsAlias() const { return m_alias_root != INVALID_ID; }
    // Aliasing can only be created from a resource once.
    bool HasAlias() const { return m_has_been_aliased; }
    uint32_t GetAliasRoot() const { return m_alias_root; }

protected:
    static constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

private:
    uint32_t m_id;
    uint32_t m_begin_pass_order = INVALID_ID;
    uint32_t m_end_pass_order = INVALID_ID; // inclusive
    // Derived resources fill this in when they resolve their own handle type.
    uint32_t m_alias_root = INVALID_ID;

    Type m_type = Type::Texture;
    bool m_is_external = false;
    RenderGraphPass* m_producer;
    std::vector<RenderGraphPass*> m_consumers;
    bool m_has_been_aliased = false;
};

} // namespace Aurora
