#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "flecs_registry.h"
#include "flecs_singleton_setter_registry.h"

struct PlayerPosition
{
    godot::Vector2 value;
};

inline FlecsRegistry register_player_components([](flecs::world& world) {
    world.component<PlayerPosition>().add(flecs::Singleton);

    register_singleton_setter("PlayerPosition", [](flecs::world& world, const godot::Dictionary& data) {
        godot::Vector2 position = data["value"];
        world.set<PlayerPosition>({ position });
    });
});
