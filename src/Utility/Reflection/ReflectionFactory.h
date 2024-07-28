#pragma once
// std include
#include <unordered_map>
#include <map>
#include <functional>
#include <string>
// thirdparty include

// Aurora include


namespace Aurora
{

class ClassField
{
public:
    ClassField(std::string&& field_type, unsigned int offset)
        : m_field_type(field_type), m_offset(offset) { }
    
    ClassField(const ClassField&) = delete;
    ClassField& operator=(const ClassField&) = delete;

    ClassField(ClassField&&) = default;
    ClassField& operator=(ClassField&&) = default;
    
    const std::string& GetFieldType() const { return m_field_type; }
    unsigned int GetOffset() const { return m_offset; }

private:
    std::string m_field_type;
    unsigned int m_offset;
};

class ReflectionFactory
{
public:
    using Constructor = std::function<void*()>;

    static ReflectionFactory& GetInstance()
    {
        static ReflectionFactory instance;
        return instance;
    }

    ReflectionFactory(const ReflectionFactory&) = delete;
    ReflectionFactory& operator=(const ReflectionFactory&) = delete;

    ReflectionFactory(ReflectionFactory&&) = delete;
    ReflectionFactory& operator=(ReflectionFactory&&) = delete;

    void RegisterField(const std::string& class_name, std::string&& field_name, ClassField&& field)
    {
        m_fields[class_name].insert(std::make_pair(std::move(field_name), std::move(field)));
    }

    const ClassField* GetField(const std::string& class_name, const std::string& field_name)
    {
        auto iter = m_fields[class_name].find(field_name);
        if (iter != m_fields[class_name].end())
        {
            return &iter->second;
        }
        return nullptr;
    }

    const std::map<std::string, ClassField>& GetFields(const std::string& class_name)
    {
        return m_fields[class_name];
    }

private:
    ReflectionFactory() = default;

private:
    std::unordered_map<std::string, std::map<std::string, ClassField>> m_fields;
};
} // namespace Aurora