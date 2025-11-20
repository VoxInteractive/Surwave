#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"


struct CharacterContactBeginDistanceSquared {
    godot::real_t value;
};

struct CharacterContactEndDistanceSquared {
    godot::real_t value;
};


inline FlecsRegistry register_game_singleton_components([](flecs::world& world) {
    const godot::real_t enemy_radius = 8.0f;
    const godot::real_t contact_margin = 2.0f;
    const godot::real_t hysteresis = 2.0f; // This creates a stable region where the enemy will remain in the Attacking state, preventing flapping.

    world.component<CharacterContactBeginDistanceSquared>("CharacterContactBeginDistanceSquared")
        .member<godot::real_t>("value")
        .add(flecs::Singleton)
        .set<CharacterContactBeginDistanceSquared>({ godot::Math::pow(enemy_radius + contact_margin, 2.0f) });

    world.component<CharacterContactEndDistanceSquared>("CharacterContactEndDistanceSquared")
        .member<godot::real_t>("value")
        .add(flecs::Singleton)
        .set<CharacterContactEndDistanceSquared>({ godot::Math::pow(enemy_radius + contact_margin + hysteresis, 2.0f) });
});
