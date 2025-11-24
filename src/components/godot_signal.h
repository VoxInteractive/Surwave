#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "src/flecs_registry.h"

struct GodotSignal {
    godot::StringName name;
    godot::Dictionary data;
};

inline FlecsRegistry register_godot_signal_component([](flecs::world& world)
{
    world.component<GodotSignal>("GodotSignal");
});
