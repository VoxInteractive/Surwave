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

struct PlayerDamageCooldown {
    godot::real_t value;
};

inline FlecsRegistry register_player_components([](flecs::world& world) {
    world.component<PlayerPosition>("PlayerPosition")
        .add(flecs::Singleton);

    world.component<PlayerDamageCooldown>("PlayerDamageCooldown")
        .member<godot::real_t>("value")
        .add(flecs::Singleton)
        .set<PlayerDamageCooldown>({ godot::real_t(0.0) });

    register_singleton_setter<godot::Vector2>("PlayerPosition", [](flecs::world& world, const godot::Vector2& player_position) {
        world.set<PlayerPosition>({ player_position });
    });

    register_singleton_getter<PlayerPosition>("PlayerPosition");
});
