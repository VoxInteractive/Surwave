#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "godot/components/variants.h"
#include "flecs/registry.h"

struct LocalTransform
{
    godot::Vector2 position;
};

inline FlecsRegistry register_character_components([](flecs::world& world)
{
    world.component<LocalTransform>()
        .member<godot::Vector2>("position");
});
