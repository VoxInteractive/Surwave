#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
    get_singleton_setters()[name] = [setter, name](flecs::world& world, const godot::Variant& v) {
        const auto expected_type = static_cast<godot::Variant::Type>(godot::GetTypeInfo<T>::VARIANT_TYPE);
        if (godot::Variant::can_convert(v.get_type(), expected_type))
        {
            setter(world, v.operator T());
        }
        else
        {
            godot::String warning_message = "Failed to set singleton component '{0}'. Cannot convert provided data from type '{1}' to the expected type '{2}'.";
            godot::UtilityFunctions::push_warning(warning_message.format(godot::Array::make(name.c_str(), godot::Variant::get_type_name(v.get_type()), godot::Variant::get_type_name(expected_type))));
        }
    };
}
