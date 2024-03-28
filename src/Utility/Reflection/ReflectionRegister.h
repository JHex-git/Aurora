#pragma once
// std include
#include <unordered_map>
// thirdparty include

// Aurora include
#include "Utility/Reflection/ReflectionFactory.h"

namespace Aurora
{
#define REFLECTABLE_IMPL(class_name, field_name, field_type) \
    class _##class_name##field_name##FieldRegister \
    { \
    public: \
        _##class_name##field_name##FieldRegister() \
        { \
            class_name _instance_class_name##Register; \
            ReflectionFactory::GetInstance().RegisterField(#class_name, #field_name, ClassField(#field_type, reinterpret_cast<uint64_t>(&(_instance_class_name##Register.field_name)) - reinterpret_cast<uint64_t>(&_instance_class_name##Register))); \
        } \
    } _instance_##class_name##field_name##FieldRegister;  

#define REFLECTABLE_DECLARE(class_name, field_name) \
    friend class _##class_name##field_name##FieldRegister;
} // namespace Aurora