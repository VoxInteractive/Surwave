// Global registry for singleton setter callbacks.
#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/variant/variant.hpp>

// Forward-declare flecs::world to avoid including the full <flecs.h> header.
// This is sufficient for using pointers or references and improves compile times.
namespace flecs
{
    class world;
}

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
