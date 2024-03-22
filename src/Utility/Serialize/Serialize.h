#pragma once
// std include
#include <unordered_map>
// thirdparty include

// Aurora include


namespace Aurora
{

class Serializable
{

};

class ObjectFactory
{
private:
    using Constructor = Serializable* (*)();

public:
    static ObjectFactory& GetInstance()
    {
        static ObjectFactory instance;
        return instance;
    }

    ObjectFactory(const ObjectFactory&) = delete;
    ObjectFactory& operator=(const ObjectFactory&) = delete;

    Constructor Create(const std::string& class_name)
    {
        auto it = m_serializable_classes.find(class_name);
        if (it != m_serializable_classes.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void Register(const std::string& class_name, Constructor class_instance)
    {
        m_serializable_classes[class_name] = class_instance;
    }

private:
    ObjectFactory() = default;

    std::unordered_map<std::string, Constructor> m_serializable_classes;
};

#define REGISTER_SERIALIZABLE(class_name) \
class_name##Register \
{ \
public: \
    class_name##Register() \
    { \
        ObjectFactory::GetInstance().Register(#class_name, &class_name##Register::CreateInstance); \
    } \
}; \
static class_name##Register class_name##_register;
} // namespace Aurora