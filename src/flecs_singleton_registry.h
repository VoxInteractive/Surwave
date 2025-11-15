#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/variant/variant.hpp>

#include <flecs.h>

// Getters

using FlecsSingletonGetter = std::function<godot::Variant(const flecs::world&)>;

namespace
{
    inline std::unordered_map<std::string, FlecsSingletonGetter>& get_singleton_getters()
    {
        static std::unordered_map<std::string, FlecsSingletonGetter> getters;
        return getters;
    }
}

template <typename T>
void register_singleton_getter(const char* name)
{
    get_singleton_getters()[name] = [](const flecs::world& world) -> godot::Variant
    {
        const T* data = world.try_get<T>();
        if (data) {
            return *data;
        }
        return godot::Variant();
    };
}

// Setters

using FlecsSingletonSetter = std::function<void(flecs::world&, const godot::Variant&)>;

inline std::unordered_map<std::string, FlecsSingletonSetter>& get_singleton_setters()
{
    static std::unordered_map<std::string, FlecsSingletonSetter> g_singleton_setters;
    return g_singleton_setters;
}

template <typename T>
void register_singleton_setter(const std::string& name, std::function<void(flecs::world&, const T&)> setter)
{
    get_singleton_setters()[name] = [setter](flecs::world& world, const godot::Variant& v) {
        setter(world, v.operator T());
    };
}
