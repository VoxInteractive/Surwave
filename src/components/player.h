#pragma once

#include <godot_cpp/variant/vector2.hpp>

#include "src/flecs_registry.h"
#include "src/flecs_singleton_registry.h"

struct PlayerPosition
{
    godot::Vector2 value;

    // This will make it implicitly convertible to a godot::Vector2, which is a type GDScript understands.
    // When get_singleton_component returns the PlayerPosition object, this conversion will be used automatically to create the godot::Variant.
    operator godot::Variant() const {
        return value;
    }
};

inline FlecsRegistry register_player_components([](flecs::world& world) {
    world.component<PlayerPosition>("PlayerPosition").add(flecs::Singleton);

    register_singleton_setter<godot::Vector2>("PlayerPosition", [](flecs::world& world, const godot::Vector2& player_position) {
        world.set<PlayerPosition>({ player_position });
    });
});
