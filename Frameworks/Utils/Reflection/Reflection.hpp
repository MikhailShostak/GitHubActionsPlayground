#pragma once

#include "Serialization/Serialization.hpp"

namespace Reflection
{

struct IMetaObject
{
public:
    const char *m_Name = nullptr;
    template<typename ClassType>
    void Serialize(const ClassType &value, Serialization::Data &data) const
    {
        auto base = static_cast<const Meta::RootType<ClassType>*>(&value);
        SerializeFunction(base, data);
    }
    template<typename ClassType>
    void Deserialize(const Serialization::Data &data, ClassType &value) const
    {
        auto base = static_cast<Meta::RootType<ClassType>*>(&value);
        DeserializeFunction(data, base);
    }
protected:
    Meta::VoidFunctionPointer<const void *, Serialization::Data &> SerializeFunction;
    Meta::VoidFunctionPointer<const Serialization::Data &, void *> DeserializeFunction;
};

inline std::unordered_map<size_t, std::unique_ptr<IMetaObject>> g_Types;

template<typename ClassType>
struct MetaObject : public IMetaObject
{
    MetaObject(const char *name)
    {
        m_Name = name;
        SerializeFunction = [](const void *object, Serialization::Data &data)
        {
            auto base = reinterpret_cast<const Meta::RootType<ClassType> *>(object);
            auto derived = static_cast<const ClassType *>(base);
            Serialization::Serialize(*derived, data);
        };
        DeserializeFunction = [](const Serialization::Data &data, void *object)
        {
            auto base = reinterpret_cast<Meta::RootType<ClassType> *>(object);
            auto derived = static_cast<ClassType *>(base);
            Serialization::Deserialize(data, *derived);
        };
    }
};

template<typename Type>
inline void Add(const char *name)
{
    g_Types.insert({ typeid(Type).hash_code(), std::make_unique<MetaObject<Type>>(name) });
}

template<typename Type>
inline void Remove()
{
    g_Types.erase(typeid(Type).hash_code());
}

template<typename Type>
inline void Serialize(const Type &value, Serialization::Data &data)
{
    const size_t hash = typeid(value).hash_code();
    auto it = g_Types.find(hash);
    if (it == g_Types.end())
    {
        return;
    }

    it->second->Serialize(value, data);
}

template<typename Type>
inline Serialization::Data Serialize(const Type &value)
{
    Serialization::Data data;
    Reflection::Serialize(value, data);
    return data;
}

template<typename Type>
inline void Deserialize(const Serialization::Data &data, Type &value)
{
    const size_t hash = typeid(value).hash_code();
    auto it = g_Types.find(hash);
    if (it == g_Types.end())
    {
        return;
    }

    it->second->Deserialize(data, value);
}

template<typename Type>
inline Type Deserialize(const Serialization::Data &data)
{
    Type value;
    Reflection::Deserialize(data, value);
    return value;
}

}

/*
class MetaObject
{
public:

    struct Property
    {
        size_t typeHash = 0;

        bool isPersistent : 1;
        bool isBoolean : 1;
        bool isInteger : 1;
        bool isFloat : 1;
        bool isEnum : 1;
        bool isClass : 1;
        bool isEmbedded : 1;

        ADelegate<AVariant(AObject *object)> getter;
        ADelegate<void(AObject *object, const AVariant &)> setter;
    };

    size_t hash = 0;
    Memory::Constructor<AObject> constructor;
    ADelegate<void(const AObject &object, AJSONObject &data)> serialize;
    ADelegate<void(const AJSONObject &data, AObject &object)> deserialize;
    AStringHashMap<Property> properties;
    MetaObject *baseObject = nullptr;

    AStringHashMap<Property> getAllProperties() const
    {
        return baseObject ? baseObject->getAllProperties() + properties : properties;
    }

    bool hasProperty(const AString &name) const
    {
        auto it = properties.find(name);
        return it != properties.end();
    }

    bool hasPropertyGetter(const AString &name) const
    {
        auto it = properties.find(name);
        return it != properties.end() && it->second.getter;
    }

    bool hasPropertySetter(const AString &name) const
    {
        auto it = properties.find(name);
        return it != properties.end() && it->second.setter;
    }

    AVariant getProperty(AObject *object, const AString &name) const
    {
        auto allProperties = getAllProperties();
        auto it = allProperties.find(name);
        if (it == allProperties.end())
        {
            return AVariant();
        }
        if (!it->second.getter)
        {
            return AVariant();
        }
        return it->second.getter(object);
    }

    void setProperty(AObject *object, const AString &name, const AVariant &value) const
    {
        auto allProperties = getAllProperties();
        auto it = allProperties.find(name);
        if (it == allProperties.end())
        {
            return;
        }
        if (!it->second.setter)
        {
            return;
        }
        it->second.setter(object, value);
    }

    template<typename OwnerType, typename ValueType, typename GetterType, typename SetterType>
    void registerMetaProperty(const AString &name, GetterType &&getter, SetterType &&setter, bool persistent = false)
    {
        using PropertyType = std::remove_reference<typename std::invoke_result<GetterType, OwnerType>::type>::type;
        using ResolvedType = std::conditional<std::is_enum<PropertyType>::value, size_t, ValueType>::type;
        properties[name] = Property{
            AMeta::getStaticTypeHash<PropertyType>(),
            persistent,
            std::is_same<PropertyType, bool>::value,
            std::is_integral<PropertyType>::value,
            std::is_floating_point<PropertyType>::value,
            std::is_enum<PropertyType>::value,
            std::is_class<PropertyType>::value,
            std::is_object<PropertyType>::value,
            getter ? ADelegate<AVariant(AObject *object)>([getter](AObject *object) { return AVariant((static_cast<OwnerType *>(object)->*getter)()); }) : nullptr,
            setter ? ADelegate<void(AObject *object, const AVariant &)>([setter](AObject *object, const AVariant &value) { if (value.isValue<ResolvedType>()) { (static_cast<OwnerType *>(object)->*setter)(ValueType(value.asValue<ResolvedType>())); } }) : nullptr,
        };
    }
};

class MetaObjectManager : public ASingleton<MetaObjectManager>
{
    AStringHashMap<MetaObject *> namedMetaObjects;
public:

    AHashMap<size_t, AStringArray> enums;
    mutable AHashMap<size_t, MetaObject *> metaObjects;

    template<typename Type>
    MetaObject &getMetaObject() const
    {
        static MetaObject object;
        static bool loaded = false;
        if (!loaded)
        {
            if constexpr (!std::is_abstract_v<Type>)
            {
                object.hash = AMeta::getStaticTypeHash<Type>();
                object.constructor = &CreateAs<Type, AObject>;
            }
            metaObjects.insert(AMeta::getStaticTypeHash<Type>(), &object);
            loaded = true;
        }
        return object;
    }

    void registerMetaObject(const AString &name, MetaObject &object)
    {
        namedMetaObjects.insert(name, &object);
    }

    void unregisterMetaObject(const AString &name)
    {
        namedMetaObjects.remove(name);
    }

    MetaObject *findMetaObject(const AString &name) const
    {
        auto it = namedMetaObjects.find(name);
        if (it == namedMetaObjects.end())
        {
            return nullptr;
        }

        return it->second;
    }
};
*/
