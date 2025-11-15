#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_setter_registry.h"

struct PlayerPosition
{
    godot::Vector2 value;
};

inline FlecsRegistry register_player_components([](flecs::world& world) {
    world.component<PlayerPosition>().add(flecs::Singleton);

    register_singleton_setter<godot::Vector2>("PlayerPosition", [](flecs::world& world, const godot::Vector2& value) {
        world.set<PlayerPosition>({ value });
    });
});
