#pragma once

#include <godot_cpp/core/math_defs.hpp>

#include "src/flecs_registry.h"

struct HitPoints {
    godot::real_t value;
};

struct MovementSpeed {
    godot::real_t value;
};

struct MeleeDamage {
    godot::real_t value;
};

struct AnimationFrameOffset {
    float value; // Even though this is an integer value, using float to avoid casting when used in shaders
};

struct DeathTimer {
    godot::real_t value;
};


inline FlecsRegistry register_enemy_stats_components([](flecs::world& world) {
    world.component<HitPoints>("HitPoints")
        .member<godot::real_t>("value");

    world.component<MeleeDamage>("MeleeDamage")
        .member<godot::real_t>("value");

    world.component<MovementSpeed>("MovementSpeed")
        .member<godot::real_t>("value");

    world.component<AnimationFrameOffset>("AnimationFrameOffset")
        .member<float>("value");

    world.component<DeathTimer>("DeathTimer")
        .member<godot::real_t>("value")
        .add(flecs::CanToggle);
});
