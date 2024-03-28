#pragma once
// std include
#include <string>
// thirdparty include

// Aurora include
#include "Utility/Reflection/ReflectionFactory.h"

namespace Aurora
{

class Reflectable
{
public:
    Reflectable() = default;
    virtual ~Reflectable() = default;

    template <typename T>
    const T& GetField(const std::string& field_name)
    {
        auto field = ReflectionFactory::GetInstance().GetField(GetClassName(), field_name);
        return *reinterpret_cast<T*>(reinterpret_cast<uint64_t>(this) + field->GetOffset());
    }

    template <typename T>
    void SetField(const std::string& field_name, const T& value)
    {
        auto field = ReflectionFactory::GetInstance().GetField(GetClassName(), field_name);
        *reinterpret_cast<T*>(reinterpret_cast<uint64_t>(this) + field->GetOffset()) = value;
    }

    const std::string& GetClassName() const { return m_class_name; }

protected:
    std::string m_class_name;
};
} // namespace Aurora